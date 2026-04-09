#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

constexpr const char* char_set = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/// @brief Returns the exact number of Base64 characters needed to encode `n` bytes.
size_t base64EncodedSize(size_t n);

/// @brief Returns the maximum number of bytes from decoding `input`.
size_t base64DecodedMaxSize(std::string_view input);

bool base64Encode(const uint8_t* input, size_t inputLen, char* output, size_t outputCap, size_t& outputLen);

bool base64Decode(std::string_view input, uint8_t* output, size_t outputCap, size_t& outputLen);