#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>
#include "CollisionEffects.hpp"

class CollisionEffectDispatcher {
    private:
        std::unordered_map<std::string, CollisionEffects::EffectFn> _effects;
    
    public:
        void registerEffect(const std::string& name, CollisionEffects::EffectFn fn);

        void execute(const std::string& name,
                        Registry& registry,
                        Entity subject,
                        Entity object,
                        FrameContext& ctx) const;

        void registerDefaults();
};