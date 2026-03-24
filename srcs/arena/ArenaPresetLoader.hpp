#pragma once

#include <string>
#include <vector>

struct ArenaPresetDefinition {
    std::string name;
    int width = 0;
    int height = 0;
    std::vector<std::string> walls;
};

class ArenaPresetLoader {
public:
    using PresetList = std::vector<ArenaPresetDefinition>;

    // Returns presets in JSON order; each preset carries its full wall mask.
    static PresetList load(const std::string& path);

    // Picks a random preset from a loaded list.
    static const ArenaPresetDefinition& pickRandom(const PresetList& presets);
};
