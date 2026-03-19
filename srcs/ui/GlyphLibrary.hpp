#pragma once

#include <unordered_map>
#include <vector>

struct GlyphCell {
    int x = 0;
    int y = 0;
};

struct GlyphDef {
    int						advance = 0;
    std::vector<GlyphCell>	cells;
};

struct GlyphLibrary {
    int									cellSize = 8;
    int									letterSpacing = 1;
    int									lineSpacing = 2;
    int                                 lineHeightCells = 0;
    std::unordered_map<char, GlyphDef>	glyphs;

	const GlyphDef* find(char c) const {
		auto it = glyphs.find(c);
		return (it == glyphs.end()) ? nullptr : &it->second;
	}

    int resolvedLineHeightCells() const {
        if (lineHeightCells > 0) {
            return lineHeightCells;
        }

        int maxCellY = 0;
        for (const auto& [_, glyph] : glyphs) {
            for (const auto& cell : glyph.cells) {
                if (cell.y > maxCellY) {
                    maxCellY = cell.y;
                }
            }
        }

        return maxCellY + 1;
    }
};