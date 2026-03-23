#include <fstream>
#include <stdexcept>
#include "CollisionRuleLoader.hpp"
#include "../../incs/third_party/json.hpp"

using json = nlohmann::json;

CollisionRuleTable CollisionRuleLoader::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("CollisionRuleLoader: cannot open file");
    }

    json data;

    try {
        file >> data;
    } catch (const json::parse_error& e) {
        throw std::runtime_error("CollisionRuleLoader: JSON parse error: " + std::string(e.what()));
    }

    CollisionRuleTable table;

    for (const auto& entry : data.at("collisionRules")) {
        CollisionRule rule;
        rule.subject = entry.at("subject").get<std::string>();
        rule.object = entry.at("object").get<std::string>();

        for (const auto& effect : entry.at("effects")) {
            rule.effects.push_back(effect.get<std::string>());
        }

        table.rules.push_back(std::move(rule));
    }

    return table;
}