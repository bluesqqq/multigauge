#pragma once

#include <initializer_list>

#include <rapidjson/document.h>

/* Rules.h
 *
 * This header is purely for PropertyMetadata's optional visibleWhen and interactableWhen.
 *
 * These exist purely to make editor UX more responsive and intuitive, as some properties
 * do not make sense in some editor states.
 *
 * The visible + interactable when getters only generate rule list JSON, and do not actually
 * evaluate conditions. It is entirely up to the target editor to resolve. See 
 * docs/schemas/rules.schema.json for returned json schema.
 */

namespace mg::rules {

rapidjson::Value makeRule(rapidjson::Document::AllocatorType& a, const char* path, const char* op, const char* value);
rapidjson::Value makeRule(rapidjson::Document::AllocatorType& a, const char* path, const char* op, std::initializer_list<const char*> values);
rapidjson::Value makeAllRule(rapidjson::Document::AllocatorType& a, std::initializer_list<rapidjson::Value> rules);
rapidjson::Value makeAnyRule(rapidjson::Document::AllocatorType& a, std::initializer_list<rapidjson::Value> rules);
rapidjson::Value makeRuleList(rapidjson::Document::AllocatorType& a, std::initializer_list<rapidjson::Value> rules);

}

//----------[ MACROS ]----------//

#define MG_UI_RULE(path_literal, op_literal, value_literal) \
    mg::rules::makeRule(a, path_literal, op_literal, value_literal)

#define MG_UI_RULE_IN(path_literal, ...) \
    mg::rules::makeRule(a, path_literal, "in", { __VA_ARGS__ })

#define MG_UI_RULE_NOT_IN(path_literal, ...) \
    mg::rules::makeRule(a, path_literal, "notIn", { __VA_ARGS__ })

#define MG_UI_RULE_ALL(...) \
    mg::rules::makeAllRule(a, { __VA_ARGS__ })

#define MG_UI_RULE_ANY(...) \
    mg::rules::makeAnyRule(a, { __VA_ARGS__ })

#define MG_UI_RULES(...) \
    mg::rules::makeRuleList(a, { __VA_ARGS__ })
