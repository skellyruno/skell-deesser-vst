#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
CXX="${CXX:-c++}"
for test in filter_smoke waveform_smoke engine_smoke band_regression engine_regression phrase_aware_regression phrase_gate_regression filter_automation_regression extreme_stress realtime_alloc_stress detector_startup_regression waveform_idle_regression block_partition_regression transparent_idle_regression quiet_detector_regression; do
  # This test overrides operator new/delete with malloc/free instrumentation.
  # GCC 15 conservatively warns on the matching delete. Isolate the warning
  # suppression to this ONE test; all other tests retain -Werror.
  extra=()
  if [[ "$test" == "realtime_alloc_stress" ]]; then extra+=(-Wno-mismatched-new-delete); fi
  "$CXX" -Ijuce_stub -std=c++17 -O2 -Wall -Wextra -Werror "${extra[@]}" "${test}.cpp" -o "/tmp/skelly_${test}"
  "/tmp/skelly_${test}"
done
