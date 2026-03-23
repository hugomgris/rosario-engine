#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include "animations/ParticleConfigLoader.hpp"

namespace {
std::string writeTempJson(const std::string& content) {
    const auto base = std::filesystem::temp_directory_path() /
        ("rosario_particle_config_test_" + std::to_string(std::rand()) + ".json");

    std::ofstream out(base);
    out << content;
    out.close();

    return base.string();
}

const MenuTrailPreset* findPresetByColor(const ParticleConfig& config, unsigned char r, unsigned char g, unsigned char b) {
    for (const auto& preset : config.menuTrails) {
        if (preset.color.r == r && preset.color.g == g && preset.color.b == b) {
            return &preset;
        }
    }
    return nullptr;
}
}

TEST(ParticleConfigLoader, ParsesMultipleMenuTrailSets) {
    const std::string path = writeTempJson(R"JSON(
{
    "dust": {
        "maxDensity": 20,
        "spawnInterval": 0.08,
        "minSize": 3.0,
        "maxSize": 15.0,
        "minLifetime": 3.0,
        "maxLifetime": 6.0
    },
    "explosion": {
        "minSize": 4.0,
        "maxSize": 18.0,
        "minLifetime": 0.3,
        "maxLifetime": 0.9,
        "minSpeed": 80.0,
        "maxSpeed": 350.0,
        "count": 28
    },
    "trail": {
        "minSize": 10.0,
        "maxSize": 20.0,
        "minLifetime": 0.2,
        "maxLifetime": 1.0,
        "minSpeed": 50.0,
        "maxSpeed": 60.0,
        "count": 3,
        "scatter": 15.0,
        "spawnInterval": 0.05
    },
    "menuTrail_yellow": {
        "direction": "LEFT",
        "color": [255, 165, 0, 255],
        "spawnInterval": 0.1,
        "x": 160,
        "y": 700
    },
    "menuTrail_blue": {
        "direction": "UP",
        "color": {"r": 70, "g": 130, "b": 180, "a": 255},
        "spawnInterval": 0.2,
        "x": 160,
        "y": 750
    }
}
)JSON");

    ParticleConfig config;
    ASSERT_NO_THROW(config = ParticleConfigLoader::load(path));

    EXPECT_EQ(config.menuTrails.size(), 2u);

    const MenuTrailPreset* yellow = findPresetByColor(config, 255, 165, 0);
    ASSERT_NE(yellow, nullptr);
    EXPECT_EQ(yellow->direction, Direction::LEFT);
    EXPECT_FLOAT_EQ(yellow->spawnInterval, 0.1f);
    EXPECT_TRUE(yellow->hasManualPosition);
    EXPECT_FLOAT_EQ(yellow->x, 160.0f);
    EXPECT_FLOAT_EQ(yellow->y, 700.0f);

    const MenuTrailPreset* blue = findPresetByColor(config, 70, 130, 180);
    ASSERT_NE(blue, nullptr);
    EXPECT_EQ(blue->direction, Direction::UP);
    EXPECT_FLOAT_EQ(blue->spawnInterval, 0.2f);
    EXPECT_TRUE(blue->hasManualPosition);
    EXPECT_FLOAT_EQ(blue->x, 160.0f);
    EXPECT_FLOAT_EQ(blue->y, 750.0f);

    std::remove(path.c_str());
}

TEST(ParticleConfigLoader, RejectsHalfManualPosition) {
    const std::string path = writeTempJson(R"JSON(
{
    "dust": {
        "maxDensity": 20,
        "spawnInterval": 0.08,
        "minSize": 3.0,
        "maxSize": 15.0,
        "minLifetime": 3.0,
        "maxLifetime": 6.0
    },
    "explosion": {
        "minSize": 4.0,
        "maxSize": 18.0,
        "minLifetime": 0.3,
        "maxLifetime": 0.9,
        "minSpeed": 80.0,
        "maxSpeed": 350.0,
        "count": 28
    },
    "trail": {
        "minSize": 10.0,
        "maxSize": 20.0,
        "minLifetime": 0.2,
        "maxLifetime": 1.0,
        "minSpeed": 50.0,
        "maxSpeed": 60.0,
        "count": 3,
        "scatter": 15.0,
        "spawnInterval": 0.05
    },
    "menuTrail_broken": {
        "direction": "RIGHT",
        "color": [255, 255, 255, 255],
        "spawnInterval": 0.1,
        "x": 160
    }
}
)JSON");

    EXPECT_THROW(
        ParticleConfigLoader::load(path),
        std::runtime_error
    );

    std::remove(path.c_str());
}
