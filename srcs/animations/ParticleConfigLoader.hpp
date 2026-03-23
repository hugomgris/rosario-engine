#pragma once

#include "ParticleConfig.hpp"
#include <iostream>

class ParticleConfigLoader {
    public:
        static ParticleConfig load(const std::string& path);
};