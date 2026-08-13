#include <multigauge/json/ArrayWriter.h>
#include <multigauge/json/Writer.h>

namespace mg::json {

bool ArrayWriter::write(bool value) noexcept { return writer_.write(value); }
bool ArrayWriter::write(int value) noexcept { return writer_.write(value); }
bool ArrayWriter::write(std::int64_t value) noexcept { return writer_.write(value); }
bool ArrayWriter::write(std::uint64_t value) noexcept { return writer_.write(value); }
bool ArrayWriter::write(float value) noexcept { return writer_.write(value); }
bool ArrayWriter::write(double value) noexcept { return writer_.write(value); }
bool ArrayWriter::write(std::string_view value) noexcept { return writer_.write(value); }
bool ArrayWriter::write(const std::string& value) noexcept { return writer_.write(value); }
bool ArrayWriter::write(const char* value) noexcept { return writer_.write(value); }
bool ArrayWriter::write(Reader value) noexcept { return writer_.write(value); }

} // namespace mg::json
