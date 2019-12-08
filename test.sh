#!/usr/bin/env bash

set -eu

run_date=$(date +'%Y-%m-%d %H:%M:%S')

meson setup out --reconfigure
ninja test -C out || true

bash -c "coredumpctl list --quiet --no-legend --no-pager --since='${run_date}'"
