#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
CXX="${CXX:-c++}"
for test in filter_smoke waveform_smoke engine_smoke band_regression engine_regression phrase_aware_regression; do
  "$CXX" -Ijuce_stub -std=c++17 -O2 -Wall -Wextra -Werror "${test}.cpp" -o "/tmp/skelly_${test}"
  "/tmp/skelly_${test}"
done
