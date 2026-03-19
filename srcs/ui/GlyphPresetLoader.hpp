#pragma once

#include <string>
#include <unordered_map>
#include "../components/PixelTextComponent.hpp"

class GlyphPresetLoader {
	public:
		using PresetTable = std::unordered_map<std::string, PixelTextComponent>;

		static PresetTable load(const std::string& path);
};
