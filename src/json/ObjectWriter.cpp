#include <multigauge/json/ObjectWriter.h>
#include <multigauge/json/Writer.h>

namespace mg::json {

bool ObjectWriter::write(std::string_view key, bool value) noexcept { return writer_.key(key) && writer_.write(value); }
bool ObjectWriter::write(std::string_view key, int value) noexcept { return writer_.key(key) && writer_.write(value); }
bool ObjectWriter::write(std::string_view key, std::int64_t value) noexcept { return writer_.key(key) && writer_.write(value); }
bool ObjectWriter::write(std::string_view key, std::uint64_t value) noexcept { return writer_.key(key) && writer_.write(value); }
bool ObjectWriter::write(std::string_view key, float value) noexcept { return writer_.key(key) && writer_.write(value); }
bool ObjectWriter::write(std::string_view key, double value) noexcept { return writer_.key(key) && writer_.write(value); }
bool ObjectWriter::write(std::string_view key, std::string_view value) noexcept { return writer_.key(key) && writer_.write(value); }
bool ObjectWriter::write(std::string_view key, const std::string& value) noexcept { return write(key, std::string_view(value)); }
bool ObjectWriter::write(std::string_view key, const char* value) noexcept { return writer_.key(key) && writer_.write(value); }
bool ObjectWriter::write(std::string_view key, Reader value) noexcept { return writer_.key(key) && writer_.write(value); }

} // namespace mg::json
