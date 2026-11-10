#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <memory>

namespace mg::io {

/// Read opens an existing file; Write creates/truncates; Append creates or writes at EOF.
enum class FileMode {
    Read,  ///< Opens an existing file
    Write, ///< Creates/truncates a file
    Append ///< Creates or writes at EOF
};

/// Exclusively owned streaming handle. Destruction closes the file.
/// The filesystem must outlive its handles. Handles are not thread-safe.
class File {
public:
    virtual ~File() = default;

    /// @brief Reads up to capacity bytes.
    /// @note On failure, `bytesRead` reports any bytes transferred.
    /// @note `data` may be nullptr only when capacity is zero. Only Read mode permits reads.
    /// @return True on success, including EOF (bytesRead == 0).
    virtual bool read(uint8_t* data, size_t capacity, size_t& bytesRead) = 0;

    /// @brief Writes all bytes or returns false.
    /// @note A partial write may occur on failure.
    /// @note `data` may be nullptr only when length is zero. Read mode rejects writes.
    virtual bool write(const uint8_t* data, size_t length) = 0;

    /// @brief Sets the absolute position within [0, file size].
    /// @note Append-mode writes always occur at EOF.
    virtual bool seek(size_t offset) = 0;

    /// @brief Returns the current file position.
    /// @note Leaves offset unchanged on failure.
    virtual bool tell(size_t& offset) = 0;

    /// @brief Flushes backend buffers.
    virtual bool flush() = 0;

    /// @brief Closes the file.
    /// @note Repeated calls succeed. All other operations fail after closing.
    virtual bool close() = 0;
};

struct CreateFileResult {
    std::unique_ptr<File> file; ///< Newly created writable file on success.
    bool alreadyExists = false; ///< The path was occupied and left untouched.
};

class FileSystem {
    public:
        virtual ~FileSystem() = default;

        //----------[ LIFECYCLE ]----------//

        virtual bool init() = 0;

        //----------[ FILES ]----------//

        virtual bool exists(const std::string& path) = 0;

        virtual bool size(const std::string& path, size_t& outSize) = 0;

        /// @brief Opens a file.
        /// @note Write and Append create missing parents.
        /// @return Unique pointer to file on success, `nullptr` on failure.
        [[nodiscard]] std::unique_ptr<File> open(const std::string& path, FileMode mode);

        /// @brief Atomically creates a new, empty writable file, creating missing parents.
        /// @return The new file, or an empty handle. On failure `alreadyExists`
        /// distinguishes an occupied path from I/O failure or lack of backend support.
        [[nodiscard]] CreateFileResult create(const std::string& path);

        /// @brief Removes a file or empty directory.
        virtual bool remove(const std::string& path) = 0;

        /// @brief Renames a file.
        virtual bool rename(const std::string& from, const std::string& to) = 0;

        //----------[ READ / WRITE ]----------//

        /// @brief Reads an entire file.
        /// @param maxBytes Maximum file-content bytes, including growth during reading.
        /// @note Returns false on size/query/I/O/allocation failure; leaves out unchanged.
        /// This bounds content size, not total process memory usage.
        bool readBytes(const std::string& path, std::vector<uint8_t>& out, size_t maxBytes);

        /// @brief Writes an entire file, creating parent directories.
        bool writeBytes(const std::string& path, const uint8_t* data, size_t length);

        /// @brief Text convenience read.
        /// @note Same content limit and failure guarantees as readBytes. Zero accepts only empty files.
        bool readText(const std::string& path, std::string& out, size_t maxBytes);

        /// @brief Text convenience write.
        bool writeText(const std::string& path, const std::string& text);

        //----------[ DIRECTORIES ]----------//

        /// @brief Makes a directory.
        virtual bool makeDirectory(const std::string& path) = 0;

        /// @brief Lists the directories under a path.
        /// @note Returns immediate child names, not full paths. Clears output on failure.
        virtual bool listDirectories(const std::string& path, std::vector<std::string>& outNames) = 0;

        /// @brief Lists the files under a path.
        /// @note Returns immediate child names, not full paths. Clears output on failure.
        virtual bool listFiles(const std::string& path, std::vector<std::string>& outNames) = 0;

    protected:
        virtual std::unique_ptr<File> openImpl(const std::string&, FileMode) = 0;
        virtual CreateFileResult createImpl(const std::string&) { return {}; }

    private:
        bool makeParentDirectories(const std::string& path);
        bool makeDirectories(const std::string& path);
};

} // namespace mg::io
