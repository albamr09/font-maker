#!/bin/bash

mkdir build && cd build
cmake .. -DBoost_INCLUDE_DIR="$BOOST_INCLUDE_DIR"
make
