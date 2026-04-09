#include <multigauge/io/Base64.h>

size_t base64EncodedSize(size_t n) { return ((n + 2) / 3) * 4; }

size_t base64DecodedMaxSize(std::string_view input) { return (input.length() / 4) * 3; }

bool base64Encode(const uint8_t *input, size_t inputLen, char *output, size_t outputCap, size_t &outputLen) {
    outputLen = 0;
    
    if ((!input && inputLen != 0) || (!output && inputLen != 0)) return false;

    size_t required = base64EncodedSize(inputLen);

    if (outputCap < required) return false;

    size_t k = 0;

    for (size_t i = 0; i < inputLen; i += 3) {
        const size_t remaining = inputLen - i;
        const size_t count = (remaining >= 3) ? 3 : remaining;

        const uint32_t b0 = input[i];
        const uint32_t b1 = (count > 1) ? input[i + 1] : 0;
        const uint32_t b2 = (count > 2) ? input[i + 2] : 0;

        const uint32_t val = (b0 << 16) | (b1 << 8) | b2;

        output[k++] = char_set[(val >> 18) & 0x3F];
        output[k++] = char_set[(val >> 12) & 0x3F];
        output[k++] = (count > 1) ? char_set[(val >> 6) & 0x3F] : '=';
        output[k++] = (count > 2) ? char_set[val & 0x3F] : '=';
    }

    outputLen = k;
    return true;
}

static int decodeBase64Char(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    if (c == '=') return -2; // padding
    return -1; // invalid
}

bool base64Decode(std::string_view input, uint8_t *output, size_t outputCap, size_t &outputLen) {
    outputLen = 0;

    if (!output && !input.empty()) return false;

    if ((input.size() % 4) != 0) return false;

    const size_t requiredMax = base64DecodedMaxSize(input);
    if (outputCap < requiredMax) return false;

    size_t k = 0;

    for (size_t i = 0; i < input.size(); i += 4) {
        const int s0 = decodeBase64Char(input[i]);
        const int s1 = decodeBase64Char(input[i + 1]);
        const int s2 = decodeBase64Char(input[i + 2]);
        const int s3 = decodeBase64Char(input[i + 3]);

        if (s0 < 0 || s1 < 0) return false;

        const bool pad2 = (s2 == -2);
        const bool pad3 = (s3 == -2);

        if (pad2) {
            if (!pad3) return false;
        } else if (s2 < 0) {
            return false;
        }


        if (!pad3 && s3 < 0) return false;

        const uint32_t v0 = static_cast<uint32_t>(s0);
        const uint32_t v1 = static_cast<uint32_t>(s1);
        const uint32_t v2 = pad2 ? 0u : static_cast<uint32_t>(s2);
        const uint32_t v3 = pad3 ? 0u : static_cast<uint32_t>(s3);

        const uint32_t val = (v0 << 18) | (v1 << 12) | (v2 << 6) | v3;

        output[k++] = static_cast<uint8_t>((val >> 16) & 0xFF);

        if (!pad2) output[k++] = static_cast<uint8_t>((val >> 8) & 0xFF);

        if (!pad3) output[k++] = static_cast<uint8_t>(val & 0xFF);

        if ((pad2 || pad3) && (i + 4 != input.size())) return false;
    }

    outputLen = k;
    return true;
}
