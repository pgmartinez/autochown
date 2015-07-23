#!/bin/sh
set -e

self_="$(readlink -f "$0")"
cd -- "${self_%/*/*}"
mkdir -p build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=debugfull #-DBUILD_TESTS=0
make
