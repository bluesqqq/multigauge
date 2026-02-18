#pragma once

#include <string>
#include <vector>

class FileSystem {
    public:
        virtual ~FileSystem() = default;

        virtual bool init() = 0;

        virtual bool readAll(const std::string& path, std::vector<uint8_t>& out) = 0;
        virtual bool writeAll(const std::string& path, const uint8_t* data, size_t length) = 0;
        
        virtual bool exists(const std::string& path) = 0;
        virtual bool size(const std::string& path, size_t& outsize) = 0;

        virtual bool remove(const std::string& path) = 0;
        virtual bool rename(const std::string& from, const std::string& to) = 0;
        virtual bool makeDirectory(const std::string& path) = 0;

        virtual bool listDirectories(const std::string& path, std::vector<std::string>& outNames) = 0;
};