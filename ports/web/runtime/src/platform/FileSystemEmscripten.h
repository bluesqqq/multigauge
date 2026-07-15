#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <multigauge/io/FileSystem.h>

class FileSystemEmscripten : public mg::io::FileSystem {
    public:
        bool init() override;

        bool readBytes(const std::string& path, std::vector<uint8_t>& out) override;
        bool writeBytes(const std::string& path, const uint8_t* data, size_t length) override;

        bool exists(const std::string& path) override;
        bool size(const std::string& path, size_t& outsize) override;

        bool remove(const std::string& path) override;
        bool rename(const std::string& from, const std::string& to) override;
        bool makeDirectory(const std::string& path) override;

        bool listDirectories(const std::string& path, std::vector<std::string>& outNames) override;
};
