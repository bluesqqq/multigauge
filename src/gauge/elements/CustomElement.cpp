#include <multigauge/gauge/elements/CustomElement.h>

namespace mg::gauge {

CustomElement::CustomElement(std::string typeId) noexcept : typeId_(std::move(typeId)) {}

const char* CustomElement::typeId() const {
    return typeId_.c_str();
}

} // namespace mg::gauge
