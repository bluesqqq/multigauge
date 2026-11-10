#pragma once
#include <multigauge/io/FileSystem.h>
#include <algorithm>
#include <limits>
#include <map>
#include <set>

namespace mg::test {
class MemoryFileSystem : public io::FileSystem {
    class MemoryFile final : public io::File {
    public:
        MemoryFile(std::vector<uint8_t>& bytes, io::FileMode mode) : bytes_(bytes), mode_(mode) {
            if (mode == io::FileMode::Write) bytes_.clear();
            if (mode == io::FileMode::Append) position_ = bytes_.size();
        }
        bool read(uint8_t* data, size_t capacity, size_t& count) override {
            count = 0;
            if (closed_ || mode_ != io::FileMode::Read || (!data && capacity)) return false;
            count = std::min(capacity, bytes_.size() - position_);
            if (count) std::copy_n(bytes_.data() + position_, count, data);
            position_ += count;
            return true;
        }
        bool write(const uint8_t* data, size_t length) override {
            if (closed_ || mode_ == io::FileMode::Read || (!data && length)) return false;
            if (mode_ == io::FileMode::Append) position_ = bytes_.size();
            if (length > std::numeric_limits<size_t>::max() - position_) return false;
            bytes_.resize(std::max(bytes_.size(), position_ + length));
            if (length) std::copy_n(data, length, bytes_.data() + position_);
            position_ += length;
            return true;
        }
        bool seek(size_t offset) override {
            if (closed_ || offset > bytes_.size()) return false;
            position_ = offset;
            return true;
        }
        bool tell(size_t& offset) override {
            if (closed_) return false;
            offset = position_;
            return true;
        }
        bool flush() override { return !closed_; }
        bool close() override { closed_ = true; return true; }
    private:
        std::vector<uint8_t>& bytes_;
        io::FileMode mode_;
        size_t position_ = 0;
        bool closed_ = false;
    };
public:
    std::map<std::string, std::vector<uint8_t>> files;
    std::set<std::string> directories;
    bool failDirectory = false;
    bool init() override { return true; }
    bool exists(const std::string& path) override { return files.contains(path) || directories.contains(path); }
    bool size(const std::string& path, size_t& out) override {
        const auto it = files.find(path);
        if (it == files.end()) return false;
        out = it->second.size();
        return true;
    }
    bool remove(const std::string& path) override { return files.erase(path) != 0 || directories.erase(path) != 0; }
    bool rename(const std::string& from, const std::string& to) override {
        const auto it = files.find(from);
        if (it == files.end()) return false;
        files[to] = std::move(it->second);
        files.erase(it);
        return true;
    }
    bool makeDirectory(const std::string& path) override {
        if (failDirectory || files.contains(path)) return false;
        directories.insert(path);
        return true;
    }
    bool listDirectories(const std::string& path, std::vector<std::string>& out) override { return listEntries(path, out, true); }
    bool listFiles(const std::string& path, std::vector<std::string>& out) override { return listEntries(path, out, false); }
protected:
    std::unique_ptr<io::File> openImpl(const std::string& path, io::FileMode mode) override {
        if (directories.contains(path) || (mode == io::FileMode::Read && !files.contains(path))) return nullptr;
        return std::make_unique<MemoryFile>(files[path], mode);
    }
private:
    bool listEntries(const std::string& path, std::vector<std::string>& out, bool dirs) {
        out.clear();
        if (!directories.contains(path)) return false;
        const std::string prefix = path + (path.empty() || path.back() == '/' ? "" : "/");
        const auto add = [&](const std::string& name) {
            if (!name.starts_with(prefix)) return;
            const std::string child = name.substr(prefix.size());
            if (!child.empty() && child.find('/') == std::string::npos) out.push_back(child);
        };
        if (dirs) for (const auto& name : directories) add(name);
        else for (const auto& entry : files) add(entry.first);
        return true;
    }
};
}
