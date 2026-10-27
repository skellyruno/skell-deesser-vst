# Skelly DeEsser V4 — UI repair pass

Based on V3.2, preserving CMake target ordering and the existing GitHub Actions workflow.

## Changes
- Replaced missing-font pictogram text in preset/navigation/sidechain buttons with JUCE vector paths (arrows, disk, menu, filter curves).
- Made sidebar section headings fit their actual component bounds at scaled window sizes.
- Enlarged frequency and threshold edit readouts; scale label fonts with the 3:2 editor window to avoid ellipses.
- Rendered the threshold cap artwork directly without the black opaque backing plate previously covering the meter.
- Enlarged the next navigation hit area and kept the full neon artwork composition.
- Left APVTS parameter IDs and DSP unchanged to retain preset compatibility.

## Validation
`bash tests/run_smoke.sh`: detector, waveform, engine tests passed on Linux using the project's test stubs.

Full JUCE VST3/Standalone Windows build, FL Studio host load/unload, font fallback on Windows, and visual alignment at all HiDPI scales require verification in a real host. No MSVC compiler or local JUCE checkout was available in this runtime.
