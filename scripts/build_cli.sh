#!/bin/bash

cd sdfglyph
mkdir build && cd build
cmake .. -DBoost_INCLUDE_DIR="$BOOST_INCLUDE_DIR"
make
mv font-maker ../
cd ../ && rm -r build
