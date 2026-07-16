#include "LittleFsFileSystem.h"

#include <FS.h>
#include <LittleFS.h>
#include <multigauge/io/Log.h>

#include <new>

bool LittleFsFileSystem::init() {
    if (LittleFS.begin(false)) {
        const size_t totalBytes = LittleFS.totalBytes();
        const size_t usedBytes = LittleFS.usedBytes();

        LOG_INFO("FS::init",
                 "LittleFS mounted: used=%u KB / total=%u KB",
                 static_cast<unsigned>(usedBytes / 1024u),
                 static_cast<unsigned>(totalBytes / 1024u));
        return true;
    }

    LOG_ERROR("LittleFsFileSystem::init", "LittleFS failed to mount");
    return false;
}

bool LittleFsFileSystem::readBytes(const std::string& path, std::vector<uint8_t>& out) {
    out.clear();

    File file = LittleFS.open(path.c_str(), "r");
    if (!file || file.isDirectory()) {
        if (file) {
            file.close();
        }
        return false;
    }

    const size_t length = file.size();

    try {
        out.resize(length);
    } catch (const std::bad_alloc&) {
        file.close();
        return false;
    }

    size_t total = 0;
    while (total < length) {
        const size_t bytesRead = file.read(out.data() + total, length - total);
        if (bytesRead == 0) {
            break;
        }
        total += bytesRead;
    }

    file.close();

    if (total != length) {
        out.clear();
        return false;
    }

    return true;
}

bool LittleFsFileSystem::writeBytes(const std::string& path, const uint8_t* data, size_t length) {
    File file = LittleFS.open(path.c_str(), "w");
    if (!file || file.isDirectory()) {
        if (file) {
            file.close();
        }
        return false;
    }

    size_t written = 0;
    while (written < length) {
        const size_t bytesWritten = file.write(data + written, length - written);
        if (bytesWritten == 0) {
            break;
        }
        written += bytesWritten;
    }

    file.close();
    return written == length;
}

bool LittleFsFileSystem::exists(const std::string& path) {
    return LittleFS.exists(path.c_str());
}

bool LittleFsFileSystem::size(const std::string& path, size_t& outsize) {
    File file = LittleFS.open(path.c_str(), "r");
    if (!file || file.isDirectory()) {
        if (file) {
            file.close();
        }
        return false;
    }

    outsize = file.size();
    file.close();
    return true;
}

bool LittleFsFileSystem::remove(const std::string& path) {
    File entry = LittleFS.open(path.c_str(), "r");
    if (entry) {
        const bool isDirectory = entry.isDirectory();
        entry.close();
        return isDirectory ? LittleFS.rmdir(path.c_str()) : LittleFS.remove(path.c_str());
    }

    return LittleFS.remove(path.c_str()) || LittleFS.rmdir(path.c_str());
}

bool LittleFsFileSystem::rename(const std::string& from, const std::string& to) {
    return LittleFS.rename(from.c_str(), to.c_str());
}

bool LittleFsFileSystem::makeDirectory(const std::string& path) {
    return LittleFS.mkdir(path.c_str());
}

bool LittleFsFileSystem::listDirectories(const std::string& path, std::vector<std::string>& outNames) {
    outNames.clear();

    File dir = LittleFS.open(path.c_str(), "r");
    if (!dir || !dir.isDirectory()) {
        if (dir) {
            dir.close();
        }
        return false;
    }

    for (File file = dir.openNextFile(); file; file = dir.openNextFile()) {
        const char* name = file.name();
        if (name != nullptr) {
            outNames.emplace_back(name);
        }
        file.close();
    }

    dir.close();
    return true;
}
