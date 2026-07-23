#include <multigauge/properties/meta/Rules.h>

static rapidjson::Value moveRulesIntoArray(rapidjson::Document::AllocatorType& a, std::initializer_list<rapidjson::Value> rules) {
    rapidjson::Value array(rapidjson::kArrayType);
    for (const auto& rule : rules) {
        rapidjson::Value copy;
        copy.CopyFrom(rule, a);
        array.PushBack(std::move(copy), a);
    }
    return array;
}

rapidjson::Value mg::rules::makeRule(rapidjson::Document::AllocatorType& a, const char* path, const char* op, const char* value) {
    rapidjson::Value rule(rapidjson::kObjectType);
    rule.AddMember("path", rapidjson::Value(path, a), a);
    rule.AddMember("op", rapidjson::Value(op, a), a);
    rule.AddMember("value", rapidjson::Value(value, a), a);
    return rule;
}

rapidjson::Value mg::rules::makeRule(rapidjson::Document::AllocatorType& a, const char* path, const char* op, std::initializer_list<const char*> values) {
    rapidjson::Value rule(rapidjson::kObjectType);
    rapidjson::Value array(rapidjson::kArrayType);

    for (const char* value : values) {
        array.PushBack(rapidjson::Value(value, a), a);
    }

    rule.AddMember("path", rapidjson::Value(path, a), a);
    rule.AddMember("op", rapidjson::Value(op, a), a);
    rule.AddMember("value", std::move(array), a);
    return rule;
}

rapidjson::Value mg::rules::makeAllRule(rapidjson::Document::AllocatorType& a, std::initializer_list<rapidjson::Value> rules) {
    rapidjson::Value group(rapidjson::kObjectType);
    group.AddMember("all", moveRulesIntoArray(a, rules), a);
    return group;
}

rapidjson::Value mg::rules::makeAnyRule(rapidjson::Document::AllocatorType& a, std::initializer_list<rapidjson::Value> rules) {
    rapidjson::Value group(rapidjson::kObjectType);
    group.AddMember("any", moveRulesIntoArray(a, rules), a);
    return group;
}

rapidjson::Value mg::rules::makeRuleList(rapidjson::Document::AllocatorType& a, std::initializer_list<rapidjson::Value> rules) {
    return moveRulesIntoArray(a, rules);
}
