#!/usr/bin/env bash

set -eu

meson setup out --reconfigure || meson setup out
meson install -C out
./out/t
