#!/usr/bin/env bash

set -eu

meson setup out --reconfigure || meson setup out
meson test -C out --print-errorlogs --gdb
