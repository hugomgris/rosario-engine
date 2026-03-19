#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct GlyphCell {
    int x = 0;
    int y = 0;
};

struct GlyphDef {
    int						advance = 0;
    int                     offsetX = 0;
    int                     offsetY = 0;
    std::vector<GlyphCell>	cells;
};

struct GlyphLibrary {
    int									cellSize = 8;
    int									letterSpacing = 1;
    int									lineSpacing = 2;
    int                                 lineHeightCells = 0;
    int                                 capHeightCells = 5;
    int                                 descenderCells = 0;
    int                                 lineGapCells = 1;
    std::unordered_map<char, GlyphDef>	glyphs;
    std::unordered_map<std::string, GlyphDef> glyphsById;

	const GlyphDef* findById(const std::string& glyphId) const {
		auto it = glyphsById.find(glyphId);
		return (it == glyphsById.end()) ? nullptr : &it->second;
	}

	const GlyphDef* find(char c) const {
		auto idIt = glyphsById.find(std::string(1, c));
		if (idIt != glyphsById.end()) {
			return &idIt->second;
		}

		auto it = glyphs.find(c);
		return (it == glyphs.end()) ? nullptr : &it->second;
	}

	int resolvedLineAdvanceCells() const {
		if (lineHeightCells > 0) {
			return lineHeightCells + lineGapCells;
		}
		return capHeightCells + lineGapCells;
	}
};