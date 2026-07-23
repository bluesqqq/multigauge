#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include <multigauge/utils/Math.h>

namespace mg::text {

class TextBuffer {
public:
    TextBuffer(
        char* data,
        size_t capacity
    ) noexcept : data(data), capacity(capacity) {
        clear();
    }

    explicit TextBuffer(std::string& dynamic) noexcept : dynamic(&dynamic) {
        clear();
    }

    void clear() noexcept {
        length = 0;
        failed = !dynamic && (!data || capacity == 0);
        if (dynamic) dynamic->clear();
        else if (!failed) data[0] = '\0';
    }

    bool append(char c) noexcept {
        if (failed) return false;

        if (dynamic) {
            dynamic->push_back(c);
            ++length; return true;
        }
        
        if (length + 1 >= capacity) {
            failed = true;
            return false;
        }
        
        data[length++] = c;
        data[length] = '\0';
        return true;
    }

    bool append(std::string_view s) noexcept {
        if (failed) return false;
        
        if (dynamic) {
            dynamic->append(s);
            length += s.size();
            return true;
        }
        
        if (s.size() > capacity - length - 1) {
            failed = true;
            return false;
        }
        
        for (char c : s) data[length++] = c;
        data[length] = '\0';
        return true;
    }

    bool appendFloat(
        float value,
        uint8_t decimals
    ) noexcept {
        char buffer[32];
        const size_t n = ::mg::utils::fastFloatToString(value, decimals, buffer, sizeof(buffer));
        return n == 0 || append({buffer, n});
    }

    bool ok() const noexcept { return !failed; }

    size_t size() const noexcept { return length; }

    std::string_view view() const noexcept {
        return dynamic 
                 ? std::string_view(*dynamic)
                 : std::string_view(data ? data : "", length);
    }

private:
    char* data = nullptr;
    std::string* dynamic = nullptr;
    size_t capacity = 0;
    size_t length = 0;
    bool failed = true;
};

}
