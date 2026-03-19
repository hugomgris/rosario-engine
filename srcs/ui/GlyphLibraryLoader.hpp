#pragma once

#include <string>
#include "GlyphLibrary.hpp"

class GlyphLibraryLoader {
    public:
        static GlyphLibrary load(const std::string& path);
};