#pragma once

#include <cstdint>
#include <string>

#include <multigauge/editor/Clipboard.h>
#include <multigauge/editor/Result.h>

namespace mg::bindings {

const char* storeString(std::string value);
const char* storeString(const char* value);
std::string toJson(const mg::editor::ClipboardSummary& summary);

}
