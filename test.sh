#!/usr/bin/env bash

set -eu

meson setup out --reconfigure || meson setup out
ninja -C out
gdb -ex run --args ./out/t
