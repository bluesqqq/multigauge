#include <multigauge/json/rj_helpers.h>

#include <multigauge/io/Logger.h>

const char *rjTypeName(const rapidjson::Value &v) {
    using rapidjson::kNullType;
    using rapidjson::kFalseType;
    using rapidjson::kTrueType;
    using rapidjson::kObjectType;
    using rapidjson::kArrayType;
    using rapidjson::kStringType;
    using rapidjson::kNumberType;

    switch (v.GetType()) {
        case kNullType:   return "null";
        case kFalseType:  return "false";
        case kTrueType:   return "true";
        case kObjectType: return "object";
        case kArrayType:  return "array";
        case kStringType: return "string";
        case kNumberType: return "number";
        default:          return "unknown";
    }
}

bool setInt(const rapidjson::Value::ConstObject &o, const char *k, int &out) {
    auto it = o.FindMember(k);
    if (it == o.MemberEnd()) return false;
    if (!it->value.IsNumber()) {
        LOG_WARN("setInt", "Key '%s' expected number but got %s", k, rjTypeName(it->value));
        return false;
    }
    out = it->value.GetInt();
    return true;
}

bool setFloat(const rapidjson::Value::ConstObject &o, const char *k, float &out) {
    auto it = o.FindMember(k);
    if (it == o.MemberEnd()) return false;
    if (!it->value.IsNumber()) {
        LOG_WARN("setFloat", "Key '%s' expected number but got %s", k, rjTypeName(it->value));
        return false;
    }
    out = it->value.GetFloat();
    return true;
}

bool setBool(const rapidjson::Value::ConstObject &o, const char *k, bool &out) {
    auto it = o.FindMember(k);
    if (it == o.MemberEnd()) return false;
    if (!it->value.IsBool()) {
        LOG_WARN("setBool", "Key '%s' expected bool but got %s", k, rjTypeName(it->value));
        return false;
    }
    out = it->value.GetBool();
    return true;
}

bool setCString(const rapidjson::Value::ConstObject &o, const char *k, const char *&out) {
    auto it = o.FindMember(k);
    if (it == o.MemberEnd()) return false;
    if (!it->value.IsString()) {
        LOG_WARN("setCString", "Key '%s' expected string but got %s", k, rjTypeName(it->value));
        return false;
    }
    out = it->value.GetString();
    return true;
}


bool setOptInt(const rapidjson::Value::ConstObject &o, const char *k, std::optional<int> &out) {
    auto it = o.FindMember(k);
    if (it == o.MemberEnd()) return false;

    const auto& v = it->value;
    if (v.IsNull()) { out.reset(); return true; }

    if (!v.IsNumber()) {
        LOG_WARN("setOptInt", "Key '%s' expected number|null but got %s", k, rjTypeName(v));
        return false;
    }
    
    out = v.GetInt();
    return true;
}

bool setOptFloat(const rapidjson::Value::ConstObject &o, const char *k, std::optional<float> &out) {
    auto it = o.FindMember(k);
    if (it == o.MemberEnd()) return false;

    const auto& v = it->value;
    if (v.IsNull()) { out.reset(); return true; }

    if (!v.IsNumber()) {
        LOG_WARN("setOptFloat", "Key '%s' expected number|null but got %s", k, rjTypeName(v));
        return false;
    }

    out = v.GetFloat();
    return true;
}

