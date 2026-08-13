#include <multigauge/json/Document.h>
#include "implementations/rapidjson.h"

namespace mg::json {

Document parse(std::string_view text) {
    auto* storage = new implementations::rapidjson::Storage;
    storage->buffer.assign(text.data(), text.size());
    storage->buffer.push_back('\0');
    storage->document.ParseInsitu(storage->buffer.data());
    if (storage->document.HasParseError()) {
        delete storage;
        return {};
    }
    return Document::adopt(implementations::rapidjson::documentBackend, storage);
}

Document object() {
    auto* storage = new implementations::rapidjson::Storage;
    storage->document.SetObject();
    storage->writer.hasRoot = true;
    return Document::adopt(implementations::rapidjson::documentBackend, storage);
}

Document array() {
    auto* storage = new implementations::rapidjson::Storage;
    storage->document.SetArray();
    storage->writer.hasRoot = true;
    return Document::adopt(implementations::rapidjson::documentBackend, storage);
}

} // namespace mg::json
