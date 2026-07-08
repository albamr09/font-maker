#include "fontstack.hpp"

#include <cstdlib>
#include <cstring>
#include <stdexcept>

extern "C" {

fontstack *create_fontstack(const char *name) {
  fontstack *f = (fontstack *)malloc(sizeof(fontstack));
  f->faces = new std::vector<FT_Face>;
  f->data = new std::vector<char *>;
  f->seen_face_names = new std::set<std::string>;

  if (name != nullptr) {
    f->name = new std::string(name);
    f->auto_name = false;
  } else {
    f->name = new std::string;
    f->auto_name = true;
  }

  FT_Library library = nullptr;
  FT_Init_FreeType(&library);

  f->library = library;
  return f;
}

void fontstack_add_face(fontstack *f, FT_Byte *base, FT_Long data_size) {
  FT_Face face = 0;
  FT_Error face_error =
      FT_New_Memory_Face(f->library, base, data_size, 0, &face);
  if (face_error) {
    throw std::runtime_error("Could not open font face");
  }
  if (face->num_faces > 1) {
    throw std::runtime_error("file has multiple faces; cowardly exiting");
  }
  if (!face->family_name) {
    throw std::runtime_error("face does not have family name");
  }
  const double scale_factor = 1.0;
  double size = 24 * scale_factor;
  FT_Set_Char_Size(face, 0, static_cast<FT_F26Dot6>(size * (1 << 6)), 0, 0);
  f->faces->push_back(face);

  if (f->auto_name) {
    std::string combined_name = std::string(face->family_name);
    if (face->style_name != NULL) {
      combined_name += " " + std::string(face->style_name);
    }

    if (f->seen_face_names->count(combined_name) == 0) {
      if (f->seen_face_names->size() > 0) {
        *f->name += ",";
      }
      *f->name += combined_name;
      f->seen_face_names->insert(combined_name);
    }
  }
}

void free_fontstack(fontstack *f) {
  for (auto fc : *f->faces) {
    FT_Done_Face(fc);
  }
  for (auto d : *f->data) {
    free(d);
  }
  FT_Done_FreeType(f->library);
  delete f->faces;
  delete f->name;
  delete f->seen_face_names;
  free(f);
}

char *fontstack_name(fontstack *f) {
  char *fname = (char *)malloc((f->name->size() + 1) * sizeof(char));
  strcpy(fname, f->name->c_str());
  return fname;
}

uint32_t fontstack_face_num_glyphs(fontstack *f, uint32_t index) {
  if (index >= f->faces->size()) {
    return 0;
  }
  return static_cast<uint32_t>(f->faces->at(index)->num_glyphs);
}

char *glyph_buffer_data(glyph_buffer *g) { return g->data; }

uint32_t glyph_buffer_size(glyph_buffer *g) { return g->size; }

void free_glyph_buffer(glyph_buffer *g) {
  free(g->data);
  free(g);
}
}
