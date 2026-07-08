#ifndef SDFGLYPH_COMPLEX_RANGE_HPP
#define SDFGLYPH_COMPLEX_RANGE_HPP

#include <cstdint>
#include <string>

#include "mapbox/glyph_foundry.hpp"

namespace sdfglyph {

// Complex-script glyphs are indexed by (glyph id + offset) instead of by unicode
// codepoint, keeping them out of the unicode plane used by regular scripts.
constexpr uint32_t COMPLEX_GLYPH_OFFSET = 0x10000; // 65536

// Builds a serialized glyphs protobuf for the 256 glyph ids starting at
// `start_glyph`, indexed by (COMPLEX_GLYPH_OFFSET + glyph id).
std::string do_glyph_range(FT_Face &face, const std::string &name,
                           uint32_t start_glyph);

} // namespace sdfglyph

#endif // SDFGLYPH_COMPLEX_RANGE_HPP
