# Skelly DeEsser V6 — VST3 / Standalone

Skelly DeEsser is a stereo de-esser with a dark neon-green gothic user interface, selectable split-band/wideband compression, frequency-targeted sidechain, A/B snapshots, peak waveform, threshold meter, and JUCE automation.

**Status:** JUCE source project; five standalone C++ tests passed with GCC and Clang. UndefinedBehaviorSanitizer tests passed with GCC. **A full JUCE/MSVC build, pluginval run, and FL Studio listening/restore test have not been run here.** Treat V6 as a development build, not a validated commercial release.

## Build

Requirements: CMake 3.22+, C++17 compiler, JUCE 8.0.12 (fetched from GitHub if not installed), MSVC (Windows) or Xcode (macOS).

Windows PowerShell:

```powershell
cmake -S . -B build -A x64 -DJUCE_PATH="C:/JUCE"  # optional JUCE_PATH
cmake --build build --config Release --parallel 2
```

macOS:

```bash
cmake -S . -B build -G Xcode -DJUCE_PATH="$HOME/JUCE"
cmake --build build --config Release --parallel 2
```

Expected VST3 bundle: `build/SkellyDeEsser_artefacts/Release/VST3/Skelly DeEsser.vst3`. The Standalone binary is in `build/SkellyDeEsser_artefacts/Release/Standalone/` (platform-dependent layout). In GitHub Actions, **Build Skelly DeEsser → Run workflow** builds the Windows VST3 and separately runs GCC + UBSan DSP regression tests on Linux.

## Controls and presets

- **Frequency (1 kHz–20 kHz)** selects the detector's sibilance region and the split-band cutoffs. Frequencies above the sample-rate-dependent safe limit are internally clamped, without changing the saved knob value.
- **Threshold (-60 to 0 dB)** sets the compression threshold, with 6 dB soft knee, ratio and range controls. The vertical meter indicates **sidechain envelope level**, not reduction.
- **Split:** parallel reduction of a complementary two-one-pole middle band; the dry waveform is bit-identical when gain reduction is 0 and output gain is 0 dB.
- **Wide:** applies the same attenuation to the full-band signal.
- **Sidechain:** optional high-pass filtering (plus Off, HPF 6, HPF 12, Band-Pass, Listen filter shapes). Changing modes smoothly crossfades the taps. Select S Chain monitor to hear the filtered detector path.
- **Settings:** Ratio, maximum reduction, attack, release, stereo link and output gain are fully automatable; settings submenus expose increase, decrease and reset.
- **A/B slots:** previous/next arrows switch snapshots stored in the host session. State is backward-compatible with older V1/V2 files.
- **Preset:** click preset title to load an external `.skelly` file, export, or initialize. The floppy icon exports a `.skelly` file. The file includes both A/B snapshots and user-interface state. Session state saves automatically with the DAW.

## DSP, realtime, and limitations

- Dynamic sibilance detector runs cascaded double-precision DF-II biquads. All stages remain warm while switching, with 6 ms tap-weight smoothing and 20 ms cutoff tracking.
- Split mode uses first-order trapezoidal (TPT) low- and high-frequency complementary stages. Summing low + middle + high yields the original sample for any cutoff. In split mode only the middle band is attenuated; frequency boundaries are gradual and **are not linear-phase**. At nonzero reduction the audible result remains IIR phase-colored, by design.
- Compression is a soft-knee static gain computer with attack/release envelope following; output, threshold and mode changes are smoothed. Accurate reported **0 samples of added algorithmic latency** (but IIR filters have frequency-dependent phase shift).
- The audio path does not allocate or perform APVTS string lookups in `processBlock`. Filter coefficients update at 16-sample intervals while center frequency is smoothed. GUI/audio waveform handoff uses atomics.
- Gain-reduction and sidechain-level displays use fast-rise, slower-release ballistic smoothing. Input waveform uses absolute stereo peak, so out-of-phase stereo is still visible.
- Source has no licensed neural or lookahead model and has not been tested against commercial processors or real vocal benchmark sets. Sound quality and heavy-automation click performance still require listening and in-host validation.

## Offline tests

Run `bash tests/run_smoke.sh` (GCC or Clang, no JUCE required). Tests include detector frequency selection, RMS envelope, waveform ring behavior, exact dry transparency, complementary-band reconstruction, mode changes, sample-rate stress and spectral amplitude comparisons. A UBSan matrix is included in GitHub Actions.

Detailed changes and measurements: [V6_DEVELOPMENT_NOTES.md](V6_DEVELOPMENT_NOTES.md). Previous release notes are included as historical references; V6 supersedes their DSP descriptions.
