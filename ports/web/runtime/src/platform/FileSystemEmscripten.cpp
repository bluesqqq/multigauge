#include "FileSystemEmscripten.h"

#include <emscripten.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>

EM_JS(void, mg_fs_init_default, (), {
    // Use MEMFS as a safe, session-scoped working filesystem.
    // The server/database remains the source of truth; JS stages content here
    // before the C++ core reads it.
    const ensureDir = (path) => {
        try {
            if (!FS.analyzePath(path).exists) {
                FS.mkdir(path);
            }
        } catch (_) {}
    };

    ensureDir('/work');
    ensureDir('/work/defaults');
    ensureDir('/work/uploads');
    ensureDir('/work/posts');
    ensureDir('/work/sessions');
});

bool FileSystemEmscripten::init() {
    mg_fs_init_default();
    return true;
}

bool FileSystemEmscripten::readBytes(const std::string& path, std::vector<uint8_t>& out) {
    out.clear();

    struct stat st;
    if (stat(path.c_str(), &st) != 0) return false;
    if (st.st_size < 0) return false;

    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return false;

    out.resize((size_t)st.st_size);
    size_t got = std::fread(out.data(), 1, out.size(), f);
    std::fclose(f);

    return got == out.size();
}

bool FileSystemEmscripten::writeBytes(const std::string& path, const uint8_t* data, size_t length) {
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) return false;

    size_t wrote = std::fwrite(data, 1, length, f);
    std::fclose(f);

    return wrote == length;
}

bool FileSystemEmscripten::exists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

bool FileSystemEmscripten::size(const std::string& path, size_t& outsize) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) return false;
    outsize = (size_t)st.st_size;
    return true;
}

bool FileSystemEmscripten::remove(const std::string& path) {
    return std::remove(path.c_str()) == 0;
}

bool FileSystemEmscripten::rename(const std::string& from, const std::string& to) {
    return std::rename(from.c_str(), to.c_str()) == 0;
}

bool FileSystemEmscripten::makeDirectory(const std::string& path) {
    if (path.empty()) return false;

    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        return (st.st_mode & S_IFDIR) != 0;
    }

    return ::mkdir(path.c_str(), 0777) == 0;
}

bool FileSystemEmscripten::listDirectories(const std::string& path, std::vector<std::string>& outNames) {
    outNames.clear();

    DIR* dir = opendir(path.c_str());
    if (!dir) return false;

    while (dirent* ent = readdir(dir)) {
        if (std::strcmp(ent->d_name, ".") == 0) continue;
        if (std::strcmp(ent->d_name, "..") == 0) continue;

        const std::string childPath = path + (path.empty() || path.back() == '/' ? "" : "/") + ent->d_name;
        struct stat st;
        if (stat(childPath.c_str(), &st) != 0) continue;
        if ((st.st_mode & S_IFDIR) == 0) continue;

        outNames.emplace_back(ent->d_name);
    }

    closedir(dir);
    return true;
}
