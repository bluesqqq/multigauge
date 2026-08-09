#pragma once

#include <multigauge/properties/Codec.h>

namespace mg {

CODEC_BEGIN(bool)
    DECODE() { return v.read(out); }
    ENCODE() { return out.write(v); }
CODEC_END()

CODEC_BEGIN(int)
    DECODE() { std::int64_t decoded; if (!v.read(decoded) || decoded < std::numeric_limits<int>::min() || decoded > std::numeric_limits<int>::max()) return false; out = static_cast<int>(decoded); return true; }
    ENCODE() { return out.write(v); }
CODEC_END()

CODEC_BEGIN(std::int8_t)
    DECODE() { std::int64_t decoded; if (!v.read(decoded) || decoded < std::numeric_limits<std::int8_t>::min() || decoded > std::numeric_limits<std::int8_t>::max()) return false; out = static_cast<std::int8_t>(decoded); return true; }
    ENCODE() { return out.write(static_cast<int>(v)); }
CODEC_END()

CODEC_BEGIN(float)
    DECODE() { double decoded; if (!v.read(decoded) || !std::isfinite(decoded) || decoded < -std::numeric_limits<float>::max() || decoded > std::numeric_limits<float>::max()) return false; out = static_cast<float>(decoded); return true; }
    ENCODE() { return out.write(v); }
CODEC_END()

CODEC_BEGIN(std::string)
    DECODE() { std::string_view decoded; if (!v.read(decoded)) return false; out.assign(decoded); return true; }
    ENCODE() { return out.write(v); }
CODEC_END()

CODEC_BEGIN_TPARAMS(typename T, std::optional<T>)
    DECODE() { if (v.isNull()) { out.reset(); return true; } T decoded{}; if (!decodeAny(v, decoded)) return false; out = std::move(decoded); return true; }
    ENCODE() { return v ? encodeAny(out, *v) : out.null(); }
CODEC_END()

CODEC_BEGIN_TPARAMS(typename T, std::vector<T>)
    DECODE() { if (!v.isArray()) return false; CodecType decoded; decoded.reserve(v.size()); for (std::size_t i = 0; i < v.size(); ++i) { T element{}; if (!decodeAny(v.element(i), element)) return false; decoded.push_back(std::move(element)); } out = std::move(decoded); return true; }
    ENCODE() { return out.writeArray([&](json::ArrayWriter&) { for (const auto& element : v) { if (!encodeAny(out, element)) return false; } return true; }); }
CODEC_END()

} // namespace mg
