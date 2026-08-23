#include <multigauge/io/FileSystem.h>

#include <multigauge/utils/Text.h>

bool mg::io::FileSystem::readText(const std::string &path, std::string &out) {
    std::vector<uint8_t> bytes;
    if (!readBytes(path, bytes)) return false;

    out.assign(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    return true;
}

bool mg::io::FileSystem::writeText(const std::string &path, const std::string &text) {
    return writeBytes(path, reinterpret_cast<const uint8_t*>(text.data()), text.size());
}

bool mg::io::FileSystem::writeBytes(const std::string& path, const uint8_t* data, size_t length) {
    const std::size_t separator = path.find_last_of("/\\");
    if (separator != std::string::npos) {
        const std::string parent = separator == 0 ? path.substr(0, 1)
            : (separator == 2 && path.size() >= 3 && path[1] == ':' ? path.substr(0, 3) : path.substr(0, separator));
        if (!makeDirectories(parent)) return false;
    }
    return writeBytesImpl(path, data, length);
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
