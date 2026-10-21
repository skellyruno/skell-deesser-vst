# Skelly DeEsser — JUCE VST3 + Standalone

V2: a stereo de-esser project based on the uploaded Skelly VST template and supplied visual reference.

## Build

Requires CMake 3.22+, JUCE 7+ (defaults to fetching JUCE 8.0.12), C++17,
Visual Studio 2022/2026 on Windows, or Xcode/Clang on macOS.

Windows:
```powershell
cmake -S . -B build -A x64 -DJUCE_PATH="C:/JUCE" # omit JUCE_PATH for auto-fetch
cmake --build build --config Release --parallel 2
```

macOS:
```bash
cmake -S . -B build -G Xcode -DJUCE_PATH="$HOME/JUCE"
cmake --build build --config Release --parallel 2
```

Expected VST3 build artifact:
`build/SkellyDeEsser_artefacts/Release/VST3/Skelly DeEsser.vst3`
Standalone: `build/SkellyDeEsser_artefacts/Release/Standalone/`.

GitHub Actions: open **Actions → Build Skelly DeEsser → Run workflow**, then download build artifact.

## Controls
- **Split**: parallel band attenuation with exact bypass response when attenuation is zero.
- **Wide**: broadband gain reduction.
- **Frequency**: sidechain target 1–20 kHz.
- **Threshold**: -60 to 0 dB; level is drawn behind draggable slider.
- **Sidechain HPF / filter menu**: Off, HPF 6, HPF 12, Band-Pass, Listen.
- **Monitor**: processed audio or detector audio.
- Host automation exposes 12 parameters; settings menu provides simple increment commands for advanced settings.
- Parameter state, preset label, active slot, and window geometry are serialized into plugin state. A/B snapshots are serialized with the host project (V2).

## Artwork
Assets are PNGs generated from user-provided design regions and procedural textures. Their boundaries are deliberately transparent where live components are drawn. They can be replaced with cleaner, original layered art without changing code.

| Asset | Pixels | Transparency |
|---|---|---|
| frame.png | 1536 × 1024 | Transparent central and panel areas |
| skull_emblem.png | 275 × 238 | Transparent outer area |
| stage_bg.png | 1140 × 645 | Opaque |
| left_panel_bg.png | 255 × 660 | Opaque |
| knob_skull.png | 108 × 110 | Transparent outside knob |
| fader_cap.png | 129 × 105 | Transparent outside cap |
| fader_housing.png | 81 × 483 | Opaque |

## Technical notes and limitations
- Uses peak envelope (not full psychoacoustic loudness matching), second-order HP/BP biquads, 2-stage HP+LP middle-band shaping and soft-knee gain computer.
- **No lookahead and no algorithmic delay:** host reports 0 samples. The IIR band shaping has frequency-dependent phase; it is not a linear-phase three-band crossover. Dry-through parallel subtraction avoids changing dry audio at zero reduction but will color nonzero attenuation.
- Sidechain filter settings update biquad coefficients on the audio thread whenever the frequency changes; large frequency sweeps can benefit from per-sample coefficient interpolation.
- The meter currently measures **sidechain envelope level**, not gain reduction; the gain reduction is indicated separately.
- V2 includes a standalone detector/envelope smoke test. Full JUCE Windows/macOS builds and listening tests are still needed. No production-grade performance or phase tests are claimed.

## V2 changes (October 2026)
- Center-frequency tuned detector in every normal filter setting (previous default HPF-only mode responded too strongly to broad high-frequency content).
- Sidechain HPF/filter modes now provide additional detector prefiltering and optional narrow-band monitoring.
- Attack/release envelope behavior retained; smoothed threshold and output gain transitions reduce zipper noise.
- A/B slot snapshots are saved in a document wrapper alongside current parameters, with backward compatibility for the original V1 serialized format.
- Slider artwork now renders inside the live knob/fader components rather than as a stationary fader-cap overlay.
- Tests: `g++ -std=c++17 -O2 tests/filter_smoke.cpp -o filter_smoke && ./filter_smoke` (does not require JUCE).

**Validation status:** standalone filter/envelope C++ smoke test passed on Linux. Full plugin compilation and DAW verification were not available in this environment. Split-mode crossover is still IIR-based and phase-colored during gain reduction; phase-neutral three-band operation is future work.

## V3: audio and UI improvements (2026-10-21)

- The waveform backdrop now renders **actual incoming audio**, captured as peak history in a single-writer/single-reader atomic ring instead of synthesizing a scrolling sine-wave pattern.
- Switching Split/Wide and Audio/Sidechain now crossfades over ~8 ms to reduce switching clicks. Both Split and Wide paths are calculated during the crossfade.
- Zero gain reduction in Split mode remains dry-transparent (parallel-subtraction structure); the mode interpolator also permits a smooth transition during mode changes.
- A stand-alone JUCE-header stub lets the engine's critical processing logic be tested without fetching or building JUCE, using `tests/run_smoke.sh`.
- CMake project version updated to 1.2.0; the original GitHub Actions build workflow has **not** been modified.

### Tests run here

`tests/run_smoke.sh` passes using C++17 compiler:

* `filter_smoke`: detector RMS 0.696 at 5.86 kHz versus 0.187 at 14 kHz and near-zero at 150 Hz.
* `waveform_smoke`: measured peak .750 from synthetic input, and no nonfinite values after a NaN sample.
* `engine_smoke`: full engine's 4096-sample dual-channel processing has measured **zero sample error when range=0** (default output gain), and engages up to 24 dB of reduction for a 5.86-kHz sine in the test.

**Limitations:** The JUCE plugin has not been built or tested in a DAW in this environment. There are no FL Studio crash/restore or audio perceptual tests yet. Split-band selection is a zero-latency parallel-filter subtraction rather than a dedicated linear-phase crossover; high-Q resonances, transitions, aliases and real vocal transparency need separate validation. The workflow still downloads JUCE during configuration unless `JUCE_PATH` points to a checkout.
