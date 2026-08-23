#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace mg::io {

class FileSystem {
    public:
        virtual ~FileSystem() = default;

        virtual bool init() = 0;

        virtual bool readBytes(const std::string& path, std::vector<uint8_t>& out) = 0;
        /// @brief Writes bytes to `path`, creating missing parent directories first.
        /// @return False when the parent path cannot be created or the backend write fails.
        bool writeBytes(const std::string& path, const uint8_t* data, size_t length);

        bool readText(const std::string& path, std::string& out);
        bool writeText(const std::string& path, const std::string& text);
        virtual bool exists(const std::string& path) = 0;
        virtual bool size(const std::string& path, size_t& outsize) = 0;

        virtual bool remove(const std::string& path) = 0;
        virtual bool rename(const std::string& from, const std::string& to) = 0;
        virtual bool makeDirectory(const std::string& path) = 0;

        virtual bool listDirectories(const std::string& path, std::vector<std::string>& outNames) = 0;

    protected:
        /// @brief Writes bytes after `writeBytes` has created the parent directory chain.
        virtual bool writeBytesImpl(const std::string& path, const uint8_t* data, size_t length) = 0;

    private:
        bool makeDirectories(const std::string& path);
};

} // namespace mg::io
