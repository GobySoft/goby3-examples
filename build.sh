#!/bin/bash

set -e -u
mkdir -p build

_cmake_flags="${GOBY3_EXAMPLES_CMAKE_FLAGS:-}"
_make_flags="${GOBY3_EXAMPLES_MAKE_FLAGS:-}"

cd build

echo "Configuring..."
(set -x; cmake .. ${_cmake_flags})
echo "Building..."
(set -x; cmake --build . -- ${_make_flags} $@)
