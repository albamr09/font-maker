#!/bin/bash
set -e

cd sdfglyph

SHARED_SOURCES="src/glyph_encode.cpp src/fontstack.cpp src/unicode_range.cpp src/complex_range.cpp"

INCLUDES="-I vendor/sdf-glyph-foundry/include/ \
	-I $BOOST_INCLUDE_DIR \
	-I vendor/protozero/include"

# Normal mode: unicode-indexed glyphs, arbitrary number of fonts.
emcc $INCLUDES \
	-s USE_FREETYPE=1 \
	-s EXPORTED_RUNTIME_METHODS=[ccall] \
	-s EXPORTED_FUNCTIONS=[_generate_glyph_buffer,_free_glyph_buffer,_create_fontstack,_free_fontstack,_fontstack_name,_glyph_buffer_data,_glyph_buffer_size,_fontstack_add_face] \
	-s ALLOW_MEMORY_GROWTH=1 \
	-o ../output/sdfglyph.js \
	-Wno-enum-constexpr-conversion \
	main.cpp $SHARED_SOURCES

# Complex mode: base font (unicode-indexed) + one complex font (glyph-indexed).
emcc $INCLUDES \
	-s USE_FREETYPE=1 \
	-s EXPORTED_RUNTIME_METHODS=[ccall] \
	-s EXPORTED_FUNCTIONS=[_generate_unicode_buffer,_generate_glyph_buffer,_free_glyph_buffer,_create_fontstack,_free_fontstack,_fontstack_name,_fontstack_face_num_glyphs,_glyph_buffer_data,_glyph_buffer_size,_fontstack_add_face] \
	-s ALLOW_MEMORY_GROWTH=1 \
	-o ../output/sdfglyph-complex.js \
	-Wno-enum-constexpr-conversion \
	complex-scripts.cpp $SHARED_SOURCES
