# Skelly DeEsser V11 — Phrase-Aware / Compact UI / VST3 / Standalone

Skelly DeEsser is a stereo de-esser with a dark neon-green gothic user interface, selectable split-band/wideband compression, frequency-targeted sidechain, A/B snapshots, peak waveform, threshold meter, and JUCE automation.

**Status:** JUCE source project. V10 passed the Windows VST3 and Standalone GitHub Actions build (run 42403563523); the new V11 changes require a fresh CI run. DSP regression/stress tests pass locally with GCC and Clang, including ASan/UBSan on the isolated engine. **Pluginval and FL Studio lifecycle, listening and project-restore tests remain pending.** Treat V11 as a development build, not a validated commercial release.

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
- **Phrase (new):** Click the **Phrase** button in Detection for Phrase-Aware processing. Reuses the same high-frequency detector and complementary split-band engine, but it does not reduce every S. A lightweight zero-lookahead tracker estimates phrase boundaries from broadband activity, measures one peak per completed sibilant event, and computes a robust within-phrase baseline. Subsequent sibilants are attenuated only when they exceed baseline by the **Phrase sensitivity** margin. In Phrase mode the existing Threshold fader becomes a bounded relative sensitivity bias, so quiet vocal recordings work without lowering an absolute threshold. A 450 ms gap in vocal activity resets the phrase; the first detected sibilant teaches the reference and passes without adaptive reduction. In **Settings → Phrase sensitivity**, lower dB is more sensitive; higher dB is more natural/forgiving (1–12 dB, default 5 dB). The tiny status field reads **PHRASE: LEARNING** or **PHRASE: READY [baseline]**. **The mode is acoustic, not a lyric/linguistic phrase recognizer; no ML, lookahead, or added delay.**
- **Split:** parallel reduction of a complementary two-one-pole middle band; the dry waveform is bit-identical when gain reduction is 0 and output gain is 0 dB.
- **Wide:** applies the same attenuation to the full-band signal.
- **Sidechain:** optional high-pass filtering (plus Off, HPF 6, HPF 12, Band-Pass, Listen filter shapes). Changing modes smoothly crossfades the taps. Select S Chain monitor to hear the filtered detector path.
- **Settings:** Ratio, maximum reduction, attack, release, stereo link, output gain and Phrase sensitivity are fully automatable; settings submenus expose increase, decrease and reset.
- **A/B slots:** previous/next arrows switch snapshots stored in the host session. State is backward-compatible with older V1/V2 files.
- **Preset:** click preset title to load an external `.skelly` file, export, or initialize. The floppy icon exports a `.skelly` file. The file includes both A/B snapshots and user-interface state. Session state saves automatically with the DAW.

## DSP, realtime, and limitations

- Dynamic sibilance detector runs cascaded double-precision DF-II biquads. All stages remain warm while switching, with 6 ms tap-weight smoothing and 20 ms cutoff tracking.
- Split mode uses first-order trapezoidal (TPT) low- and high-frequency complementary stages. Summing low + middle + high yields the original sample for any cutoff. In split mode only the middle band is attenuated; frequency boundaries are gradual and **are not linear-phase**. At nonzero reduction the audible result remains IIR phase-colored, by design.
- Compression is a soft-knee static gain computer with attack/release envelope following; output, threshold and mode changes are smoothed. Accurate reported **0 samples of added algorithmic latency** (but IIR filters have frequency-dependent phase shift).
- The audio path does not allocate or perform APVTS string lookups in `processBlock`. Filter coefficients update at 16-sample intervals while center frequency is smoothed. GUI/audio waveform handoff uses atomics.
- Gain-reduction and sidechain-level displays use fast-rise, slower-release ballistic smoothing. Input waveform uses absolute stereo peak, so out-of-phase stereo is still visible.
- The Phrase-Aware reference is ephemeral per running audio stream and is not stored in DAW presets; switching modes keeps the tracker warm for seamless transitions. The existing `detectMode` parameter stays **exactly Split (0) / Wide (1)** for compatibility with old automation. The new `phraseAware` automatable Boolean selects the third mode without remapping saved choice values.
- Source has no licensed neural or lookahead model and has not been tested against commercial processors or real vocal benchmark sets. Sound quality and heavy-automation click performance still require listening and in-host validation.

## Offline tests

Run `bash tests/run_smoke.sh` (GCC or Clang, no JUCE required). Tests include detector frequency selection, RMS envelope, waveform ring behavior, exact dry transparency, complementary-band reconstruction, mode changes, sample-rate stress, phrase segmentation, quiet vs loud sibilance comparisons, outlier immunity, and phrase reset. A UBSan matrix is included in GitHub Actions.

Detailed changes and measurements: [V6_DEVELOPMENT_NOTES.md](V6_DEVELOPMENT_NOTES.md). Previous release notes are included as historical references; V6 supersedes their DSP descriptions.

## V7 compact interface

The default editor size is **633 x 422** (3:2), approximately **45% smaller in each dimension** than the V6 default of 1152 x 768. The editor remains resizable with a 540 x 360 minimum and 1536 x 1024 maximum. **Settings → Window size** offers Compact (55%), Medium (70%), Large (85%), and Original (100%). UI size changes persist in the project state. Saved V6 editor widths are migrated to the compact default once on first opening the V7 editor, then normal user resizing is preserved.

UI fixes: scaled section header fonts and ornaments; fixed overlapping NEXT/preset buttons; removed double-rendered skull artwork; darkened busy grunge behind controls; adjusted readout lettering and silent waveform visibility. All changes are in JUCE code, without generating new image mockups. The V6 DSP implementation and all seven original V5 image assets are preserved.

**Build status:** Standalone DSP regression tests are included. Full JUCE/MSVC and host visual tests remain necessary.

## V8 Phrase-Aware development notes

See [V8_PHRASE_AWARE_NOTES.md](V8_PHRASE_AWARE_NOTES.md) for algorithm, controls, known trade-offs, and measured synthetic results. Default editor remains 633 × 422 (45% smaller in each dimension than the original 1152 × 768). No new image assets were added.


## V10 DSP and UI improvements

See [V10_ENGINEERING_NOTES.md](V10_ENGINEERING_NOTES.md): lower realtime CPU load during static settings, smoothed ratio/range/stereo link automation, and more readable compact UI controls.

## V9 reliability updates

- Smoothed crossfades between Split, Wide, and Phrase use convex weighting during simultaneous host automation.
- Phrase event gate hysteresis avoids overcounting wavering sibilants when learning a phrase; the existing UI now shows whether the phrase is still learning or has a baseline.
- GitHub Actions now packages Standalone separately from VST3 and runs GCC, Clang, UBSan, and UI geometry checks.
- See [V9_ENGINEERING_NOTES.md](V9_ENGINEERING_NOTES.md) for test scope and limitations.


## V11 stress and pre-release gate

The source tree now has a deterministic hostile-host stress test (11 sample rates, 0–8192 sample blocks, corrupt samples/parameters, mono/stereo, mode switching) and a DSP allocation test. Run with `bash tests/run_smoke.sh`; CI also runs AddressSanitizer and UndefinedBehaviorSanitizer. See [RELEASE_READINESS.md](RELEASE_READINESS.md) for the outstanding release gates and [V11_ENGINEERING_NOTES.md](V11_ENGINEERING_NOTES.md) for what was and was not verified.
## V12 maintenance

- Restored sidechain monitor filtering takes effect on the first sample; later filter changes still crossfade.
- Idle waveform rendering stops issuing redraw requests after the visible history reaches silence.
- Preset import rejects empty/corrupt parameter state without replacing active settings.
- Expanded first-buffer monitor and waveform-idle regression checks in the existing CI job.

See [V12_ENGINEERING_NOTES.md](V12_ENGINEERING_NOTES.md).

## V12.2 host validation

The Windows workflow now runs pinned pluginval v1.0.4 at strictness level 5 after compiling the VST3. This is a **CI-only** validator with no new plugin runtime dependency. New buffer partition regression tests compare identical input across 1–8192 sample host blocks. Buttons have a visible focus outline when selected from the keyboard.

## Permanent SkellCore headless JUCE UI-renderer workflow

Development builds can opt into `SkellyDeEsserUIRenderer` via
`-DSKELLY_DEESSER_BUILD_UI_RENDERER=ON`. Normal VST3 and Standalone releases
do **not** fetch or link SkellCore.

- **Pinned SkellCore commit:** `438bc74a86a40bf54cfa8089832a477822c8ccda`
- **JUCE:** existing `8.0.12` pin
- **True editor:** `SkellyDeEsserAudioProcessorEditor` instantiated with the
  real `SkellyDeEsserAudioProcessor` and its actual APVTS and DSP engine.
- **CI:** `.github/workflows/ui-renderer.yml` runs on UI code/assets/CMake
  changes, pull requests and manual dispatch. It uploads per-scene PNGs,
  a contact sheet and `manifest.json`. Failures stop the screenshot job.
- **Scenes:** minimum 540×360, default 633×422, medium 806×537,
  large 979×653, maximum 1536×1024; Split/Wide/Phrase,
  sidechain monitoring, HPF, A/B preset, disabled button,
  frequency/threshold extremes, and keyboard focus.
- **Interaction smoke checks:** actual JUCE mode/monitor buttons, fader,
  knob and A/B controls must change their real processor parameter values.
- **Layout checks:** all visible direct child components stay inside the
  editor; image captures must be nonblank; important modes and fader positions
  must produce distinct image hashes.

Run locally on a Windows machine with CMake and supported Visual Studio:
```powershell
cmake -S . -B build-ui -A x64 -DSKELLY_DEESSER_BUILD_UI_RENDERER=ON
cmake --build build-ui --config Release --target SkellyDeEsserUIRenderer
.\build-ui\Release\SkellyDeEsserUIRenderer.exe ui-screenshots
```

**Workflow rule:** After any meaningful UI change, repeat:
BUILD → RENDER → INSPECT ACTUAL PNGs → FIX → RENDER AGAIN.
The PNG renderer does not certify OS-level popup menus or DAW-specific
high-DPI/window behavior; those need separate real-host visual checks.
The renderer is intentionally not embedded into the released VST3.
