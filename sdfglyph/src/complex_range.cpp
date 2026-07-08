#include "complex_range.hpp"

#include <protozero/pbf_writer.hpp>

#include "glyph_encode.hpp"

namespace sdfglyph {

namespace {

void do_glyph(protozero::pbf_writer &parent, FT_Face &face, FT_UInt glyph_id) {
  // Skip glyph IDs that don't exist in this face.
  if (glyph_id >= static_cast<FT_UInt>(face->num_glyphs))
    return;

  // The glyph ID stored in the PBF is the FONT GLYPH ID + offset, not the
  // unicode codepoint.
  encode_glyph(parent, face, glyph_id, COMPLEX_GLYPH_OFFSET + glyph_id);
}

} // namespace

std::string do_glyph_range(FT_Face &face, const std::string &name,
                           uint32_t start_glyph) {
  std::string fontstack_data;
  {
    protozero::pbf_writer fontstack(fontstack_data);

    uint32_t start = COMPLEX_GLYPH_OFFSET + start_glyph;
    uint32_t end = start + 255;

    fontstack.add_string(1, name);
    fontstack.add_string(2, std::to_string(start) + "-" + std::to_string(end));

    for (uint32_t gid = start_glyph;
         gid < start_glyph + 256 &&
         gid < static_cast<uint32_t>(face->num_glyphs);
         gid++) {
      do_glyph(fontstack, face, gid);
    }
  }

  std::string glyphs_data;
  {
    protozero::pbf_writer glyphs(glyphs_data);
    glyphs.add_message(1, fontstack_data);
  }
  return glyphs_data;
}

} // namespace sdfglyph
