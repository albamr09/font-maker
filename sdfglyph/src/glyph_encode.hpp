#ifndef SDFGLYPH_GLYPH_ENCODE_HPP
#define SDFGLYPH_GLYPH_ENCODE_HPP

#include <cstdint>

#include "mapbox/glyph_foundry.hpp"
#include <protozero/pbf_writer.hpp>

namespace sdfglyph {

// Renders the glyph at `glyph_index` of `face` as an SDF and appends the encoded
// glyph protobuf message to `parent`. `stored_id` is written to field 1 and is
// either a unicode codepoint (unicode-indexed mode) or a glyph index plus an
// offset (glyph-indexed / complex mode).
void encode_glyph(protozero::pbf_writer &parent, FT_Face face,
                  FT_UInt glyph_index, uint32_t stored_id);

} // namespace sdfglyph

#endif // SDFGLYPH_GLYPH_ENCODE_HPP
