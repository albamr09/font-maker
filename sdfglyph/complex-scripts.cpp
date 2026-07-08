#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "src/complex_range.hpp"
#include "src/fontstack.hpp"
#include "src/unicode_range.hpp"

#ifndef EMSCRIPTEN
#include "ghc/filesystem.hpp"

// allow font filenames with commas
#define CXXOPTS_VECTOR_DELIMITER '\0'
#include "cxxopts.hpp"
#endif
#include <fstream>
#include <iostream>

using namespace std;

extern "C" {
// Unicode-indexed glyphs come only from the base face (index 0).
glyph_buffer *generate_unicode_buffer(fontstack *f, uint32_t start_codepoint) {
  std::vector<FT_Face> base_face{f->faces->at(0)};
  string result = sdfglyph::do_range(base_face, *f->name, start_codepoint,
                                     start_codepoint + 255);

  glyph_buffer *g = (glyph_buffer *)malloc(sizeof(glyph_buffer));
  char *result_ptr = (char *)malloc(result.size());
  result.copy(result_ptr, result.size());
  g->data = result_ptr;
  g->size = result.size();
  return g;
}

// Glyph-indexed glyphs come from the complex face (index 1).
glyph_buffer *generate_glyph_buffer(fontstack *f, uint32_t start_glyph) {
  string result = sdfglyph::do_glyph_range(f->faces->at(1), *f->name, start_glyph);

  glyph_buffer *g = (glyph_buffer *)malloc(sizeof(glyph_buffer));
  char *result_ptr = (char *)malloc(result.size());
  result.copy(result_ptr, result.size());
  g->data = result_ptr;
  g->size = result.size();
  return g;
}
}

#ifndef EMSCRIPTEN
int main(int argc, char *argv[]) {
  cxxopts::Options cmd_options(
      "complex-font-maker",
      "Create complex-script font PBFs from a base font and one complex font.");
  cmd_options.add_options()(
      "output", "Output directory (to be created, must not already exist)",
      cxxopts::value<string>())(
      "fonts", "Input fonts: exactly a base font followed by one complex font",
      cxxopts::value<vector<string>>())(
      "name", "Override output fontstack name", cxxopts::value<string>())(
      "help", "Print usage");
  cmd_options.positional_help("<OUTPUT_DIR> <BASE_FONT> <COMPLEX_FONT>");
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

  // Complex mode operates on exactly two fonts: a base font (unicode-indexed)
  // and one complex-script font (glyph-indexed).
  if (fonts.size() != 2) {
    cout << "ERROR: complex-font-maker requires exactly 2 fonts (a base font "
            "and one complex font), got "
         << fonts.size() << "." << endl;
    cout << cmd_options.help() << endl;
    exit(1);
  }

  if (ghc::filesystem::exists(output_dir)) {
    cout << "ERROR: output directory " << output_dir << " exists." << endl;
    exit(1);
  }
  if (ghc::filesystem::exists(output_dir))
    ghc::filesystem::remove_all(output_dir);
  ghc::filesystem::create_directory(output_dir);

  fontstack *f = create_fontstack(
      result.count("name") ? result["name"].as<string>().c_str() : nullptr);

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

  std::cout << "Generating unicode indexed glyphs" << std::endl;

  // Unicode based indexing (for latin scripts) from the base face.
  for (int i = 0; i < 65536; i += 256) {
    glyph_buffer *g = generate_unicode_buffer(f, i);
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

  std::cout << "Generating glyph indexed glyphs" << std::endl;

  // Glyph based indexing (for complex scripts) from the complex face.
  FT_Face complexFace = f->faces->at(1);
  for (FT_UInt gid = 0; gid < static_cast<FT_UInt>(complexFace->num_glyphs);
       gid += 256) {
    glyph_buffer *g = generate_glyph_buffer(f, gid);

    char *data = glyph_buffer_data(g);
    uint32_t buffer_size = glyph_buffer_size(g);

    uint32_t start = sdfglyph::COMPLEX_GLYPH_OFFSET + gid;
    uint32_t end = start + 255;

    std::ofstream output;
    std::string outname = output_dir + "/" + fname + "/" + std::to_string(start) +
                          "-" + std::to_string(end) + ".pbf";

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
