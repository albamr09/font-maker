#ifndef SDFGLYPH_FONTSTACK_HPP
#define SDFGLYPH_FONTSTACK_HPP

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "mapbox/glyph_foundry.hpp"

struct fontstack {
  FT_Library library;
  std::vector<FT_Face> *faces;
  std::vector<char *> *data;
  std::set<std::string> *seen_face_names;
  std::string *name;
  bool auto_name;
};

struct glyph_buffer {
  char *data;
  uint32_t size;
};

extern "C" {

// Lifecycle / metadata helpers shared by every mode. Passing a null `name` lets
// the fontstack auto-derive its name from the added faces.
fontstack *create_fontstack(const char *name);
void fontstack_add_face(fontstack *f, FT_Byte *base, FT_Long data_size);
void free_fontstack(fontstack *f);
char *fontstack_name(fontstack *f);

// Number of glyphs in the face at `index`, or 0 when out of range. Used by the
// complex WASM worker to know how many glyph-indexed ranges to request.
uint32_t fontstack_face_num_glyphs(fontstack *f, uint32_t index);

char *glyph_buffer_data(glyph_buffer *g);
uint32_t glyph_buffer_size(glyph_buffer *g);
void free_glyph_buffer(glyph_buffer *g);
}

#endif // SDFGLYPH_FONTSTACK_HPP
