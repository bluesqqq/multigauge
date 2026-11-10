#include <multigauge/io/FileSystem.h>

#include <multigauge/utils/Text.h>
#include <new>

bool mg::io::FileSystem::readBytes(const std::string& path, std::vector<uint8_t>& out, size_t maxBytes) {
    try {
        size_t fileSize = 0;
        if (!size(path, fileSize) || fileSize > maxBytes) return false;
        auto file = open(path, FileMode::Read);
        if (!file) return false;

        std::vector<uint8_t> buffer;
        if (fileSize > buffer.max_size()) return false;
        buffer.reserve(fileSize);
        uint8_t chunk[512];
        while (true) {
            size_t bytesRead = 0;
            if (!file->read(chunk, sizeof(chunk), bytesRead)) return false;
            if (bytesRead == 0) break;
            if (bytesRead > sizeof(chunk) || bytesRead > maxBytes - buffer.size()) return false;
            buffer.insert(buffer.end(), chunk, chunk + bytesRead);
        }
        if (!file->close()) return false;
        out = std::move(buffer);
        return true;
    } catch (const std::bad_alloc&) {
        return false;
    }
}

bool mg::io::FileSystem::writeBytes(const std::string& path, const uint8_t* data, size_t length) {
    if (!data && length) return false;
    auto file = open(path, FileMode::Write);
    if (!file) return false;

    const bool written = file->write(data, length);
    const bool closed = file->close();

    return written && closed;
}


bool mg::io::FileSystem::readText(const std::string &path, std::string &out, size_t maxBytes) {
    std::vector<uint8_t> bytes;
    if (!readBytes(path, bytes, maxBytes)) return false;

    if (bytes.size() > out.max_size()) return false;
    try {
        if (bytes.empty()) out.clear();
        else out.assign(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    } catch (const std::bad_alloc&) {
        return false;
    }
    return true;
}

bool mg::io::FileSystem::writeText(const std::string &path, const std::string &text) {
    return writeBytes(path, reinterpret_cast<const uint8_t*>(text.data()), text.size());
}

std::unique_ptr<mg::io::File> mg::io::FileSystem::open(const std::string& path, FileMode mode) {
    if (mode != FileMode::Read && mode != FileMode::Write && mode != FileMode::Append) return nullptr;
    if (mode != FileMode::Read && !makeParentDirectories(path)) return nullptr;
    return openImpl(path, mode);
}

bool mg::io::FileSystem::makeParentDirectories(const std::string& path) {
    const std::size_t separator = path.find_last_of("/\\");
    if (separator != std::string::npos) {
        const std::string parent = separator == 0 ? path.substr(0, 1)
            : (separator == 2 && path.size() >= 3 && path[1] == ':' ? path.substr(0, 3) : path.substr(0, separator));
        if (!makeDirectories(parent)) return false;
    }
    return true;
}

bool mg::io::FileSystem::makeDirectories(const std::string& path) {
    if (path.empty() || exists(path)) return true;

    std::string current;
    std::size_t index = 0;
    if (path.size() >= 2 && path[1] == ':') {
        current = path.substr(0, 2);
        index = 2;
        if (index < path.size() && utils::isPathSeparator(path[index])) {
            current.push_back(path[index]);
            ++index;
        }
    } else if (utils::isPathSeparator(path[0])) {
        current.push_back(path[0]);
        index = 1;
    }

    while (index < path.size()) {
        while (index < path.size() && utils::isPathSeparator(path[index])) ++index;
        if (index >= path.size()) break;

        const std::size_t end = path.find_first_of("/\\", index);
        const std::string segment = path.substr(index, end == std::string::npos ? std::string::npos : end - index);
        if (segment.empty()) return false;
        if (!current.empty() && !utils::isPathSeparator(current.back())) current.push_back('/');
        current.append(segment);
        if (!exists(current) && !makeDirectory(current)) return false;
        if (end == std::string::npos) break;
        index = end + 1;
    }

    return true;
}
