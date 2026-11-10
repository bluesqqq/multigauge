#pragma once
#include <multigauge/io/FileSystem.h>
#include <algorithm>
#include <limits>
#include <map>
#include <optional>
#include <set>

namespace mg::test {
class MemoryFileSystem : public io::FileSystem {
    class MemoryFile final : public io::File {
    public:
        MemoryFile(MemoryFileSystem& fs, std::vector<uint8_t>& bytes, io::FileMode mode)
            : fs_(fs), bytes_(bytes), mode_(mode) {
            if (mode == io::FileMode::Write) bytes_.clear();
            if (mode == io::FileMode::Append) position_ = bytes_.size();
        }
        ~MemoryFile() override { close(); }
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
            const size_t accepted = std::min(length, fs_.writeBudget.value_or(length));
            if (accepted > std::numeric_limits<size_t>::max() - position_) return false;
            bytes_.resize(std::max(bytes_.size(), position_ + accepted));
            if (accepted) std::copy_n(data, accepted, bytes_.data() + position_);
            position_ += accepted;
            if (fs_.writeBudget) *fs_.writeBudget -= accepted;
            return accepted == length;
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
        bool flush() override {
            ++fs_.flushCalls;
            return !closed_ && !fs_.failFlush;
        }
        bool close() override {
            if (closed_) return true;
            closed_ = true;
            ++fs_.closeCalls;
            return !fs_.failClose;
        }
    private:
        MemoryFileSystem& fs_;
        std::vector<uint8_t>& bytes_;
        io::FileMode mode_;
        size_t position_ = 0;
        bool closed_ = false;
    };
public:
    std::map<std::string, std::vector<uint8_t>> files;
    std::set<std::string> directories;
    bool failDirectory = false;
    bool failOpen = false;
    bool failRemove = false;
    bool failFlush = false;
    bool failClose = false;
    std::optional<size_t> writeBudget;
    size_t flushCalls = 0;
    size_t closeCalls = 0;
    bool init() override { return true; }
    bool exists(const std::string& path) override { return files.contains(path) || directories.contains(path); }
    bool size(const std::string& path, size_t& out) override {
        const auto it = files.find(path);
        if (it == files.end()) return false;
        out = it->second.size();
        return true;
    }
    bool remove(const std::string& path) override { return !failRemove && (files.erase(path) != 0 || directories.erase(path) != 0); }
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
        if (failOpen || directories.contains(path) || (mode == io::FileMode::Read && !files.contains(path))) return nullptr;
        return std::make_unique<MemoryFile>(*this, files[path], mode);
    }
    io::CreateFileResult createImpl(const std::string& path) override {
        if (failOpen) return {};
        if (directories.contains(path)) return {nullptr, true};
        const auto [entry, inserted] = files.try_emplace(path);
        if (!inserted) return {nullptr, true};
        return {std::make_unique<MemoryFile>(*this, entry->second, io::FileMode::Write)};
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
