#include "GlyphPresetLoader.hpp"
#include "../../incs/third_party/json.hpp"
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

namespace {
	GameState parseGameState(const std::string& value) {
		if (value == "Menu") return GameState::Menu;
		if (value == "Playing") return GameState::Playing;
		if (value == "Paused") return GameState::Paused;
		if (value == "GameOver") return GameState::GameOver;
		if (value == "Exiting") return GameState::Exiting;
		throw std::runtime_error("GlyphPresetLoader: unknown GameState: " + value);
	}

	Color parseColor(const json& node) {
		if (!node.is_array() || node.size() != 4) {
			throw std::runtime_error("GlyphPresetLoader: color must be [r,g,b,a]");
		}

		return Color{
			static_cast<unsigned char>(node.at(0).get<int>()),
			static_cast<unsigned char>(node.at(1).get<int>()),
			static_cast<unsigned char>(node.at(2).get<int>()),
			static_cast<unsigned char>(node.at(3).get<int>())
		};
	}
}

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
		preset.centerX = entry.value("centerX", false);
		preset.visible = entry.value("visible", false);

		if (entry.contains("color") && entry.at("color").is_array() && entry.at("color").size() == 4) {
			preset.color = parseColor(entry.at("color"));
		}

		if (entry.contains("glyphOverrides") && entry.at("glyphOverrides").is_array()) {
			for (const auto& override : entry.at("glyphOverrides")) {
				size_t index = override.at("index").get<size_t>();
				std::string glyphId = override.at("glyph").get<std::string>();
				preset.glyphOverrides[index] = glyphId;
			}
		}

		if (entry.contains("glyphLigatures") && entry.at("glyphLigatures").is_array()) {
			for (const auto& ligature : entry.at("glyphLigatures")) {
				size_t index = ligature.at("index").get<size_t>();
				GlyphLigatureOverride override;
				override.length = ligature.value("length", static_cast<size_t>(2));
				override.glyph = ligature.at("glyph").get<std::string>();
				preset.glyphLigatures[index] = override;
			}
		}

		if (entry.contains("glyphColorOverrides") && entry.at("glyphColorOverrides").is_array()) {
			for (const auto& override : entry.at("glyphColorOverrides")) {
				size_t index = override.at("index").get<size_t>();
				preset.glyphColorOverrides[index] = parseColor(override.at("color"));
			}
		}

		if (entry.contains("glyphCellColorOverrides") && entry.at("glyphCellColorOverrides").is_array()) {
			for (const auto& glyphOverride : entry.at("glyphCellColorOverrides")) {
				size_t index = glyphOverride.at("index").get<size_t>();
				std::vector<GlyphCellColorOverride> cells;

				if (glyphOverride.contains("cells") && glyphOverride.at("cells").is_array()) {
					for (const auto& cellNode : glyphOverride.at("cells")) {
						GlyphCellColorOverride cellOverride;
						cellOverride.x = cellNode.at("x").get<int>();
						cellOverride.y = cellNode.at("y").get<int>();
						cellOverride.color = parseColor(cellNode.at("color"));
						cells.push_back(cellOverride);
					}
				}

				preset.glyphCellColorOverrides[index] = std::move(cells);
			}
		}

		if (entry.contains("visibleInStates") && entry.at("visibleInStates").is_array()) {
			for (const auto& stateNode : entry.at("visibleInStates")) {
				preset.visibleInStates.push_back(parseGameState(stateNode.get<std::string>()));
			}
		}

		table[preset.id] = preset;
	}

	return table;
}
