#pragma once

#include <string>
#include "CollisionRule.hpp"

class CollisionRuleLoader {
    public:
        static CollisionRuleTable load(const std::string& path);
};