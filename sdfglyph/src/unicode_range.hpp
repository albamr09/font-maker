#ifndef SDFGLYPH_UNICODE_RANGE_HPP
#define SDFGLYPH_UNICODE_RANGE_HPP

#include <string>
#include <vector>

#include "mapbox/glyph_foundry.hpp"

namespace sdfglyph {

// Builds a serialized glyphs protobuf for the unicode codepoints [start, end].
// The first face that contains a glyph for a given codepoint wins.
std::string do_range(std::vector<FT_Face> &faces, const std::string &name,
                     unsigned start, unsigned end);

} // namespace sdfglyph

#endif // SDFGLYPH_UNICODE_RANGE_HPP
