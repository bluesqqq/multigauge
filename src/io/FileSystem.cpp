#include <multigauge/io/FileSystem.h>

bool mg::io::FileSystem::readText(const std::string &path, std::string &out) {
    std::vector<uint8_t> bytes;
    if (!readBytes(path, bytes)) return false;

    out.assign(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    return true;
}

bool mg::io::FileSystem::writeText(const std::string &path, const std::string &text) {
    return writeBytes(path, reinterpret_cast<const uint8_t*>(text.data()), text.size());
}
