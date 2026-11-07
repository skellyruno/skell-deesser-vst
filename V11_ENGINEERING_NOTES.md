# Skelly DeEsser V11 — hostile-host DSP hardening and UI refinements

## Improvements

- **Preset-correct first block:** Ratio, range, threshold, output gain, stereo link, Split/Wide/Phrase blend, monitor blend, and frequency start from the active settings on the first audio block after `prepareToPlay`. Prevents initial milliseconds of unwanted reduction when opening a project with maximum reduction = 0. Later automation stays smoothed.
- **Non-finite parameter sanitization:** Threshold, frequency, ratio, reduction range, attack, release, link, output, and phrase sensitivity validated once per block with a safe default for NaN/Inf. Integer modes are bounded. `std::clamp` alone does not sanitize NaN.
- **Corrupted sample safety:** NaN/Inf sample inputs are replaced by silence, and extreme finite input/output (> ±1,000,000) is bounded solely to prevent filter/output overflow. No clipping occurs at normal floating-point DAW levels.
- **Invalid sample-rate fallback:** Extremely malformed sample rates use 48 kHz to avoid invalid smoothing/filter state.
- **UI:** More readable Phrase learning text at compact size; distinct detector-level label on the central rail; host-driven mode buttons repaint immediately; non-finite typed parameter edits rejected. No new artwork and no change to default 633×422 design.
- **Tests:** `tests/extreme_stress.cpp` covers 11 sample rates (8k to 768k), 3850 hostile processing blocks totaling 2,859,714 synthetic samples, 0/1/2/…/8192-sample blocks, mono/stereo, invalid settings, DC, silence, impulses, denormals, out-of-phase signals, non-finite and huge audio, hostile mode and monitor switching. Also verifies strict zero-reduction transparency in all three modes. `tests/realtime_alloc_stress.cpp` tracks allocations within 700 DSP processing blocks.

## Local results

- GCC and Clang smoke/engine tests: passed.
- Clang AddressSanitizer+UndefinedBehaviorSanitizer on extreme stress: passed.
- GCC UBSan on realtime-allocation stress: passed.
- Zero heap allocations inside the isolated DSP engine `process()` during 700 test blocks.
- Performance duration in stress output is informational and machine-dependent, **not a worst-case deadline guarantee**.

## Not established by these results

- No real vocal corpus or blind listening tests; synthetic S bursts are not enough to validate naturalness.
- No FL Studio host lifecycle, VST3 scan/close/reopen, preset file import/export or pluginval assertion from this test.
- No complete C++ plugin allocations measurement (only isolated engine tests), no realtime worst-case latency under heavy OS load.
- No perceptual verification of the IIR split-band phase/coloration or quality of Phrase segmentation with breath noise and background music.

See the staged tasks in `RELEASE_READINESS.md`.