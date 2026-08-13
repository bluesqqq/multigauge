#pragma once

#include <string>

#include <multigauge/json/Reader.h>
#include <multigauge/json/Writer.h>

namespace mg::json {

struct DocumentBackend {
    void (*destroy)(void*) noexcept;
    Reader (*root)(const void*) noexcept;
    Writer (*writer)(void*) noexcept;
    std::string (*toString)(const void*);
};

} // namespace mg::json
