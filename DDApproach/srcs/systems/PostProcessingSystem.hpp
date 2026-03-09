#ifndef POSTPROCESSINGSYSTEM_HPP
#define POSTPROCESSINGSYSTEM_HPP

#include "ecs/Registry.hpp"
#include "graphics/Renderer.hpp"
#include "data/GameConfig.hpp"

class PostProcessingSystem {
public:
    PostProcessingSystem(const GameConfig& config);
    void applyPostProcessing(const Registry& registry, const Renderer& renderer);

private:
    const GameConfig& gameConfig;
    void applyBloomEffect();
    void applyBlurEffect();
    void applyColorCorrection();
};

#endif // POSTPROCESSINGSYSTEM_HPP