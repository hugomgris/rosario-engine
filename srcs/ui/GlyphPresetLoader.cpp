#include "GlyphPresetLoader.hpp"
#include "../../incs/third_party/json.hpp"
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

GlyphPresetLoader::PresetTable GlyphPresetLoader::load(const std::string& path) {
	std::ifstream file(path);
	if (!file.is_open())
		throw std::runtime_error("GlyphPresetLoader: cannot open: " + path);

	json data;
	try {
		file >> data;
	} catch (const json::parse_error& e) {
		throw std::runtime_error("GlyphPresetLoader: parse error: " + std::string(e.what()));
	}

	PresetTable table;

	for (const auto& entry : data.at("presets")) {
		PixelTextComponent preset;
		preset.id = entry.at("id").get<std::string>();
		preset.text = entry.at("text").get<std::string>();
		preset.position = {
			entry.at("x").get<float>(),
			entry.at("y").get<float>()
		};
		preset.scale = entry.value("scale", 1.0f);
		preset.visible = entry.value("visible", false);

		if (entry.contains("color") && entry.at("color").is_array() && entry.at("color").size() == 4) {
			const auto& c = entry.at("color");
			preset.color = Color{
				static_cast<unsigned char>(c.at(0).get<int>()),
				static_cast<unsigned char>(c.at(1).get<int>()),
				static_cast<unsigned char>(c.at(2).get<int>()),
				static_cast<unsigned char>(c.at(3).get<int>())
			};
		}

		table[preset.id] = preset;
	}

	return table;
}
