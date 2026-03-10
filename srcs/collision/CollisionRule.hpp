#pragma once

#include <string>
#include <vector>

struct CollisionRule {
    std::string                 subject;
    std::string                 object;
    std::vector<std::string>    effects;
};

struct CollisionRuleTable {
    std::vector<CollisionRule> rules;

    const CollisionRule* find(const std::string& subject, const std::string& object) const {
        for (const auto& rule : rules) {
            if (rule.subject == subject && rule.object == object) {
                return &rule;
            }
        }

        return nullptr;
    }
};