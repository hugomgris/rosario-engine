// PostProcessingSystem.cpp
#include "PostProcessingSystem.hpp"
#include "Renderer.hpp"
#include "GameConfig.hpp"

PostProcessingSystem::PostProcessingSystem(Renderer& renderer)
    : renderer(renderer) {}

void PostProcessingSystem::applyPostProcessing(const GameConfig& config) {
    // Apply post-processing effects based on the configuration
    if (config.enableBloom) {
        applyBloom();
    }
    if (config.enableBlur) {
        applyBlur();
    }
    if (config.enableColorGrading) {
        applyColorGrading();
    }
}

void PostProcessingSystem::applyBloom() {
    // Implementation of bloom effect
}

void PostProcessingSystem::applyBlur() {
    // Implementation of blur effect
}

void PostProcessingSystem::applyColorGrading() {
    // Implementation of color grading effect
}