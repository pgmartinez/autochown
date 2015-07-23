#!/bin/sh
set -e

self_="$(readlink -f "$0")"
cd -- "${self_%/*/*}"
rm -r build/
