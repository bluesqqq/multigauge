#include <cstdint>

namespace mg::io {

class Time {
    public:
        virtual ~Time() = default;
        virtual uint64_t getMillis() = 0;
        virtual uint64_t getMicros() = 0;
};

} // namespace mg::io
