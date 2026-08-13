#pragma once

#include <string_view>
#include <utility>

#include <multigauge/json/DocumentBackend.h>

namespace mg::json {

class Document {
public:
    Document() = default;
    Document(const Document&) = delete;
    Document& operator=(const Document&) = delete;
    Document(Document&& other) noexcept : backend_(other.backend_), storage_(std::exchange(other.storage_, nullptr)) {}
    Document& operator=(Document&& other) noexcept {
        if (this != &other) {
            reset();
            backend_ = other.backend_;
            storage_ = std::exchange(other.storage_, nullptr);
        }
        return *this;
    }
    ~Document() { reset(); }

    [[nodiscard]] bool valid() const noexcept { return backend_ && storage_; }
    [[nodiscard]] Reader root() const noexcept { return valid() ? backend_->root(storage_) : Reader{}; }
    [[nodiscard]] Writer writer() noexcept { return valid() ? backend_->writer(storage_) : Writer{}; }
    [[nodiscard]] std::string toString() const { return valid() ? backend_->toString(storage_) : std::string{}; }

    static Document adopt(const DocumentBackend& backend, void* storage) noexcept { return Document(&backend, storage); }

private:
    Document(const DocumentBackend* backend, void* storage) noexcept : backend_(backend), storage_(storage) {}
    void reset() noexcept {
        if (storage_) backend_->destroy(storage_);
        storage_ = nullptr;
        backend_ = nullptr;
    }

    const DocumentBackend* backend_ = nullptr;
    void* storage_ = nullptr;
};

Document parse(std::string_view text);
Document object();
Document array();

} // namespace mg::json
