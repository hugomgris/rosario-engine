#include <fstream>
#include <stdexcept>
#include <cctype>
#include "ButtonConfigLoader.hpp"
#include "../../incs/third_party/json.hpp"

using json = nlohmann::json;

namespace {
Color parseColor(const json& entry, const char* key) {
	const auto& raw = entry.at(key);
	return Color{
		static_cast<unsigned char>(raw.at(0).get<int>()),
		static_cast<unsigned char>(raw.at(1).get<int>()),
		static_cast<unsigned char>(raw.at(2).get<int>()),
		static_cast<unsigned char>(raw.at(3).get<int>())
	};
}

std::string makeIdFromLabel(const std::string& label) {
	std::string id;
	id.reserve(label.size());

	bool prevUnderscore = false;
	for (unsigned char c : label) {
		if (std::isalnum(c)) {
			id.push_back(static_cast<char>(std::tolower(c)));
			prevUnderscore = false;
		} else if (!prevUnderscore) {
			id.push_back('_');
			prevUnderscore = true;
		}
	}

	while (!id.empty() && id.front() == '_') id.erase(id.begin());
	while (!id.empty() && id.back() == '_') id.pop_back();

	if (id.empty()) id = "button";
	return id;
}

ButtonConfig parseButtonConfig(const json& entry, ButtonMenu menu) {
	ButtonConfig cfg;

	cfg.label   = entry.at("label").get<std::string>();
	cfg.id      = entry.contains("id")
		? entry.at("id").get<std::string>()
		: makeIdFromLabel(cfg.label);
	cfg.menu    = menu;
	cfg.index   = entry.at("index").get<int>();
	cfg.height  = entry.at("height").get<float>();
	cfg.width   = entry.at("width").get<float>();
	cfg.verticalPositionFactor = entry.at("verticalPositionFactor").get<float>();
	cfg.fontSize = entry.contains("fontSize")
		? entry.at("fontSize").get<float>()
		: cfg.fontSize;
	cfg.outlineThickness = entry.contains("outlineThickness")
		? entry.at("outlineThickness").get<float>()
		: cfg.outlineThickness;
	cfg.verticalSpacing = entry.contains("verticalSpacing")
		? entry.at("verticalSpacing").get<float>()
		: cfg.verticalSpacing;

	cfg.outlineColor      = parseColor(entry, "outlineColor");
	cfg.backgroundColor   = parseColor(entry, "backgroundColor");
	cfg.textColor         = parseColor(entry, "textColor");
	cfg.hoverColor        = parseColor(entry, "hoverColor");
	cfg.outlineHoverColor = parseColor(entry, "outlineHoverColor");
	cfg.textHoverColor    = parseColor(entry, "textHoverColor");

	return cfg;
}
}

ButtonConfigLoader::ButtonTable ButtonConfigLoader::load(const std::string& path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		throw std::runtime_error("ButtonConfigLoader: cannot open file");
	}

	json data;

	try {
		file >> data;
	} catch (const json::parse_error& e) {
		throw std::runtime_error("ButtonConfigLoader: JSON parse error: " + std::string(e.what()));
	}

	ButtonTable table;

	for (const auto& entry : data.at("start")) {
		ButtonConfig cfg = parseButtonConfig(entry, ButtonMenu::Start);
		table.start[cfg.id] = cfg;
	}

	for (const auto& entry : data.at("gameover")) {
		ButtonConfig cfg = parseButtonConfig(entry, ButtonMenu::GameOver);
		table.gameOver[cfg.id] = cfg;
	}

	return table;
}