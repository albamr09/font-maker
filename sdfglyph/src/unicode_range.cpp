#include "unicode_range.hpp"

#include <limits>
#include <stdexcept>

#include <protozero/pbf_writer.hpp>

#include "glyph_encode.hpp"

namespace sdfglyph {

namespace {

void do_codepoint(protozero::pbf_writer &parent, std::vector<FT_Face> &faces,
                  FT_ULong char_code) {
  for (auto const &face : faces) {
    FT_UInt char_index = FT_Get_Char_Index(face, char_code);
    if (char_index > 0) {
      // shortening conversion
      if (char_code > std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("Invalid value for char_code: too large");
      }
      encode_glyph(parent, face, char_index, static_cast<uint32_t>(char_code));
      return;
    }
  }
}

} // namespace

std::string do_range(std::vector<FT_Face> &faces, const std::string &name,
                     unsigned start, unsigned end) {
  std::string fontstack_data;
  {
    protozero::pbf_writer fontstack{fontstack_data};

    fontstack.add_string(1, name);
    fontstack.add_string(2, std::to_string(start) + "-" + std::to_string(end));

    for (unsigned x = start; x <= end; x++) {
      do_codepoint(fontstack, faces, x);
    }
  }

  std::string glyphs_data;
  {
    protozero::pbf_writer glyphs{glyphs_data};
    glyphs.add_message(1, fontstack_data);
  }
  return glyphs_data;
}

} // namespace sdfglyph
