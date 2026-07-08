#!/bin/bash

emcc -I vendor/sdf-glyph-foundry/include/ \
		 -I "$BOOST_INCLUDE_DIR" \
		 -I vendor/protozero/include \
		 -s USE_FREETYPE=1 \
		 -s EXPORTED_RUNTIME_METHODS=[ccall] \
		 -s EXPORTED_FUNCTIONS=[_generate_glyph_buffer,_free_glyph_buffer,_create_fontstack,_free_fontstack,_fontstack_name,_glyph_buffer_data,_glyph_buffer_size,_fontstack_add_face] \
		 -s ALLOW_MEMORY_GROWTH=1 \
		 -o output/sdfglyph.js \
		 -Wno-enum-constexpr-conversion \
		 main.cpp
