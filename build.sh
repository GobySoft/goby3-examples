#!/bin/bash

if [ -z "${GOBY3_EXAMPLES_CMAKE_FLAGS}" ]; then
    GOBY3_EXAMPLES_CMAKE_FLAGS=
fi

if [ -z "${GOBY3_EXAMPLES_MAKE_FLAGS}" ]; then
    GOBY3_EXAMPLES_MAKE_FLAGS=
fi

set -e -u
mkdir -p build

echo "Configuring..."
echo "cmake .. ${GOBY3_EXAMPLES_CMAKE_FLAGS}"
pushd build >& /dev/null
cmake .. ${GOBY3_EXAMPLES_CMAKE_FLAGS}
echo "Building..."
echo "make ${GOBY3_EXAMPLES_MAKE_FLAGS} $@"
make ${GOBY3_EXAMPLES_MAKE_FLAGS} $@
popd >& /dev/null
