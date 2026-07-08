#define BOOST_NO_CXX98_FUNCTION_BASE
// workaround unary_function in boost::geometry

#include "glyph_encode.hpp"

// glyph_foundry_impl.hpp provides RenderSDF (non-inline definitions), so it must
// be included in exactly one translation unit across the whole binary.
#include "mapbox/glyph_foundry_impl.hpp"

#include <limits>
#include <stdexcept>
#include <string>

namespace sdfglyph {

void encode_glyph(protozero::pbf_writer &parent, FT_Face face,
                  FT_UInt glyph_index, uint32_t stored_id) {
  sdf_glyph_foundry::glyph_info glyph;
  glyph.glyph_index = glyph_index;
  sdf_glyph_foundry::RenderSDF(glyph, 24, 3, 0.25, face);

  std::string glyph_data;
  protozero::pbf_writer glyph_message{glyph_data};

  // direct type conversions, no need for checking or casting
  glyph_message.add_uint32(3, glyph.width);
  glyph_message.add_uint32(4, glyph.height);
  glyph_message.add_sint32(5, glyph.left);

  glyph_message.add_uint32(1, stored_id);

  // node-fontnik uses glyph.top - glyph.ascender, assuming that the baseline
  // will be based on the ascender. However, Mapbox/MapLibre shaping assumes
  // a baseline calibrated on DIN Pro w/ ascender of ~25 at 24pt
  int32_t top = glyph.top - 25;
  glyph_message.add_sint32(6, top);

  // double to uint
  if (glyph.advance < std::numeric_limits<uint32_t>::min() ||
      glyph.advance > std::numeric_limits<uint32_t>::max()) {
    throw std::runtime_error("Invalid value for glyph.advance");
  } else {
    glyph_message.add_uint32(7, static_cast<uint32_t>(glyph.advance));
  }

  if (glyph.width > 0) {
    glyph_message.add_bytes(2, glyph.bitmap);
  }

  parent.add_message(3, glyph_data);
}

} // namespace sdfglyph
