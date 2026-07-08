#define BOOST_NO_CXX98_FUNCTION_BASE
// workaround unary_function in boost::geometry
#include "mapbox/glyph_foundry.hpp"
#include "mapbox/glyph_foundry_impl.hpp"
#include <cstdint>
#include <protozero/pbf_writer.hpp>

#ifndef EMSCRIPTEN
#include "ghc/filesystem.hpp"

// allow font filenames with commas
#define CXXOPTS_VECTOR_DELIMITER '\0'
#include "cxxopts.hpp"
#endif
#include <iostream>

using namespace std;

void do_glyph(protozero::pbf_writer &parent, std::vector<FT_Face> &faces,
              FT_UInt glyph_id) {

  for (auto const &face : faces) {

    // Skip glyph IDs that don't exist in this face.
    if (glyph_id >= static_cast<FT_UInt>(face->num_glyphs))
      continue;

    sdf_glyph_foundry::glyph_info glyph;
    glyph.glyph_index = glyph_id;
    sdf_glyph_foundry::RenderSDF(glyph, 24, 3, 0.25, face);

    std::string glyph_data;
    protozero::pbf_writer glyph_message{glyph_data};

    // direct type conversions, no need for checking or casting
    glyph_message.add_uint32(3, glyph.width);
    glyph_message.add_uint32(4, glyph.height);
    glyph_message.add_sint32(5, glyph.left);

    // IMPORTANT:
    // The glyph ID stored in the PBF is now the FONT GLYPH ID,
    // not the Unicode code point.
    glyph_message.add_uint32(1, glyph_id);

    // node-fontnik uses glyph.top - glyph.ascender, assuming that the
    // baseline will be based on the ascender. However, Mapbox/MapLibre
    // shaping assumes a baseline calibrated on DIN Pro w/ ascender of ~25 at
    // 24pt
    int32_t top = glyph.top - 25;
    if (top < numeric_limits<int32_t>::min() ||
        top > numeric_limits<int32_t>::max()) {
      throw runtime_error("Invalid value for glyph.top-25");
    } else {
      glyph_message.add_sint32(6, top);
    }

    // double to uint
    if (glyph.advance < numeric_limits<uint32_t>::min() ||
        glyph.advance > numeric_limits<uint32_t>::max()) {
      throw runtime_error("Invalid value for glyph.top-glyph.ascender");
    } else {
      glyph_message.add_uint32(7, static_cast<uint32_t>(glyph.advance));
    }

    if (glyph.width > 0) {
      glyph_message.add_bytes(2, glyph.bitmap);
    }

    parent.add_message(3, glyph_data);
    return;
  }
}

std::string do_range(std::vector<FT_Face> &faces, std::string name,
                     unsigned start, unsigned end) {

  std::string fontstack_data;
  {
    protozero::pbf_writer fontstack{fontstack_data};

    fontstack.add_string(1, name);
    fontstack.add_string(2, std::to_string(start) + "-" + std::to_string(end));

    for (unsigned gid = start; gid <= end; ++gid) {
      do_glyph(fontstack, faces, gid);
    }
  }

  std::string glyphs_data;
  {
    protozero::pbf_writer glyphs{glyphs_data};
    glyphs.add_message(1, fontstack_data);
  }
  return glyphs_data;
}

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
  FT_Error error = FT_Init_FreeType(&library);

  f->library = library;
  return f;
}

void fontstack_add_face(fontstack *f, FT_Byte *base, FT_Long data_size) {
  FT_Face face = 0;
  FT_Error face_error =
      FT_New_Memory_Face(f->library, base, data_size, 0, &face);
  if (face_error) {
    throw runtime_error("Could not open font face");
  }
  if (face->num_faces > 1) {
    throw runtime_error("file has multiple faces; cowardly exiting");
  }
  if (!face->family_name) {
    throw runtime_error("face does not have family name");
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

glyph_buffer *generate_glyph_buffer(fontstack *f, uint32_t start_glyph) {

  string result = do_range(*f->faces, *f->name, start_glyph, start_glyph + 255);
  glyph_buffer *g = (glyph_buffer *)malloc(sizeof(glyph_buffer));
  char *result_ptr = (char *)malloc(result.size());
  result.copy(result_ptr, result.size());
  g->data = result_ptr;
  g->size = result.size();
  return g;
}

char *glyph_buffer_data(glyph_buffer *g) { return g->data; }

uint32_t glyph_buffer_size(glyph_buffer *g) { return g->size; }

void free_glyph_buffer(glyph_buffer *g) {
  free(g->data);
  free(g);
}
}

#ifndef EMSCRIPTEN
int main(int argc, char *argv[]) {
  cxxopts::Options cmd_options("font-maker",
                               "Create font PBFs from TTFs or OTFs.");
  cmd_options.add_options()(
      "output", "Output directory (to be created, must not already exist)",
      cxxopts::value<string>())("fonts", "Input font(s) (as TTF or OTF)",
                                cxxopts::value<vector<string>>())(
      "name", "Override output fontstack name",
      cxxopts::value<string>())("help", "Print usage");
  cmd_options.positional_help("<OUTPUT_DIR> <INPUT_FONT> [INPUT_FONT2 ...]");
  cmd_options.parse_positional({"output", "fonts"});
  auto result = cmd_options.parse(argc, argv);
  if (result.count("help")) {
    cout << cmd_options.help() << endl;
    exit(0);
  }
  if (result.count("output") == 0 || result.count("fonts") == 0) {
    cout << cmd_options.help() << endl;
    exit(1);
  }
  auto output_dir = result["output"].as<string>();
  auto fonts = result["fonts"].as<vector<string>>();

  if (ghc::filesystem::exists(output_dir)) {
    cout << "ERROR: output directory " << output_dir << " exists." << endl;
    exit(1);
  }
  if (ghc::filesystem::exists(output_dir))
    ghc::filesystem::remove_all(output_dir);
  ghc::filesystem::create_directory(output_dir);

  fontstack *f = create_fontstack(result["name"].as<string>().c_str());

  for (auto const &font : fonts) {
    std::ifstream file(font, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    char *buffer = (char *)malloc(size);
    f->data->push_back(buffer);
    file.read(buffer, size);
    std::cout << "Adding " << font << std::endl;
    fontstack_add_face(f, (FT_Byte *)buffer, size);
  }

  std::string fname{fontstack_name(f)};

  ghc::filesystem::create_directory(output_dir + "/" + fname);

  FT_Long maxGlyphs = 0;
  for (auto face : *f->faces) {
    maxGlyphs = std::max(maxGlyphs, face->num_glyphs);
  }

  for (FT_UInt i = 0; i < static_cast<FT_UInt>(maxGlyphs); i += 256) {
    glyph_buffer *g = generate_glyph_buffer(f, i);
    char *data = glyph_buffer_data(g);
    uint32_t buffer_size = glyph_buffer_size(g);

    ofstream output;
    std::string outname = output_dir + "/" + fname + "/" + to_string(i) + "-" +
                          to_string(i + 255) + ".pbf";
    output.open(outname);
    output.write(data, buffer_size);
    output.close();

    std::cout << "Wrote " << outname << std::endl;

    free_glyph_buffer(g);
  }

  free_fontstack(f);

  return 0;
}
#endif
