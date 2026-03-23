#include "GlyphLibraryLoader.hpp"
#include <fstream>
#include <stdexcept>
#include "../../incs/third_party/json.hpp"

using json = nlohmann::json;

GlyphLibrary GlyphLibraryLoader::load(const std::string& path) {
	std::ifstream file(path);
	if (!file.is_open()) throw std::runtime_error("Failed to open glyph file: " + path);

	json data;
	file >> data;

	GlyphLibrary lib;
	if (data.contains("defaults")) {
		const auto& d = data["defaults"];
		lib.cellSize = d.value("cellSize", lib.cellSize);
		lib.letterSpacing = d.value("letterSpacing", lib.letterSpacing);
		lib.lineSpacing = d.value("lineSpacing", lib.lineSpacing);
		lib.lineHeightCells = d.value("lineHeightCells", lib.lineHeightCells);
		lib.capHeightCells = d.value("capHeightCells", lib.capHeightCells);
		lib.descenderCells = d.value("descenderCells", lib.descenderCells);
		lib.lineGapCells = d.value("lineGapCells", lib.lineGapCells);
	}

	for (auto it = data["glyphs"].begin(); it != data["glyphs"].end(); ++it) {
		const std::string key = it.key();
		if (key.empty()) continue;

		GlyphDef def;
		def.advance = it.value().value("advance", 0);
		def.offsetX = it.value().value("offsetX", 0);
		def.offsetY = it.value().value("offsetY", 0);

		for (const auto& c : it.value()["cells"]) {
			def.cells.push_back({ c.at(0).get<int>(), c.at(1).get<int>() });
		}

		if (key.size() == 1) {
			lib.glyphs[key[0]] = def;
		}
		lib.glyphsById[key] = std::move(def);
	}
	return lib;
}