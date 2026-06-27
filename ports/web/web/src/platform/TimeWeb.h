#pragma once
#include <multigauge/io/Time.h>

class TimeWeb : public mg::io::Time {
    public:
        uint64_t getMillis() override;
        uint64_t getMicros() override;
};
