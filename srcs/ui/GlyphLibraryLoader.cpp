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
	}

	for (auto it = data["glyphs"].begin(); it != data["glyphs"].end(); ++it) {
		const std::string key = it.key();
		if (key.empty()) continue;
		const char ch = key[0];

		GlyphDef def;
		def.advance = it.value().value("advance", 0);

		for (const auto& c : it.value()["cells"]) {
			def.cells.push_back({ c.at(0).get<int>(), c.at(1).get<int>() });
		}
		lib.glyphs[ch] = std::move(def);
	}
	return lib;
}