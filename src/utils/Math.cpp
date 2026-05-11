#include <multigauge/utils/Math.h>

namespace mg::utils {

size_t fastFloatToString(float value, uint8_t decimalPlaces, char* buf, size_t bufSize) {
    if (bufSize == 0) return 0;

    char* p = buf;

    if (value < 0.0f) {
        if ((size_t)(p - buf) < bufSize - 1) *p++ = '-';
        value = -value;
    }

    float rounding = 0.5f;
    for (uint8_t i = 0; i < decimalPlaces; i++) rounding /= 10.0f;

    value += rounding;

    int32_t intPart = (int32_t)value;
    float fracPart = value - (float)intPart;

    char intBuf[12];
    char* q = intBuf + sizeof(intBuf) - 1;
    *q = '\0';

    if (intPart == 0) {
        *--q = '0';
    } else {
        while (intPart > 0) {
            *--q = char('0' + (intPart % 10));
            intPart /= 10;
        }
    }

    while (*q && (size_t)(p - buf) < bufSize - 1) {
        *p++ = *q++;
    }

    if (decimalPlaces > 0 && (size_t)(p - buf) < bufSize - 1) {
        *p++ = '.';
    }

    for (uint8_t i = 0; i < decimalPlaces; i++) {
        fracPart *= 10.0f;
        int digit = (int)fracPart;
        if ((size_t)(p - buf) < bufSize - 1) {
            *p++ = char('0' + digit);
        }
        fracPart -= digit;
    }

    *p = '\0';
    return (size_t)(p - buf);
}

std::string floatToString(float value, uint8_t decimalPlaces) {
    char buf[16];
    fastFloatToString(value, decimalPlaces, buf, sizeof(buf));
    return std::string(buf);
}

} // namespace mg::utils
