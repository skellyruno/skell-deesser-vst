# V6 engineering update — October 27, 2026

## Fixed or improved

1. **New complementary mid-band DSP:** `ComplementaryBand.h` uses two topology-preserving one-pole stages. `low + mid + high` equals the input sample by construction, even when automation changes cutoff. Split mode reduces only mid; Wide reduces full signal. Zero reduction remains unchanged.
2. **Smoother sidechain transitions:** `SidechainFilter.h` runs all taps continuously (no stale IIR state) and crossfades detector selection using 6 ms one-pole tap-weight smoothing. Frequency updates use 20 ms center smoothing with coefficient refresh every 16 samples. Nyquist-safe cutoff clamps also support 8 kHz sessions.
3. **Responsive meters:** attack/release ballistics applied to published sidechain and gain-reduction values, independent of buffer size. Waveform correctly shows strongest stereo peak rather than a mid signal that may cancel in antiphase.
4. **Lower realtime overhead:** audio processor caches all APVTS raw-value pointers during construction and reads atomics per audio block. No per-block parameter ID hashing or heap allocations introduced.
5. **Preset files:** `.skelly` binary state import/export via asynchronous file chooser, in addition to host state and A/B snapshots. Rejects missing, malformed, or larger-than-4-MiB preset files. Preset title menu includes load, export, init.
6. **Better advanced settings:** six value-labelled submenus with increase, decrease and reset; values are snapped to declared APVTS ranges and sent as host automation gestures. Improved logo weight and backdrop visibility; new 24-LED gain-reduction readout complements the sidechain-level meter.
7. **CI regression job:** GCC and UndefinedBehaviorSanitizer tests run in the Linux CI job alongside the existing Windows VST3 build. V6 project version bumped to 1.6.0.

## Synthetic test measurements (48 kHz)

With dual-tone input (150 Hz amplitude 0.25; 5860 Hz amplitude 0.30), ratio 12, maximum reduction 18 dB, threshold -48 dB, attack 0.1 ms:

| Measurement | Split | Wide |
|---|---:|---:|
| Recovered 150 Hz amplitude | 0.24960 | 0.03147 |
| Recovered 5860 Hz amplitude | 0.08903 | 0.03777 |

Split preserves ~99.8% of the low-frequency tone's amplitude in this synthetic test while reducing the 5.86 kHz tone. Measurements are NOT a human listening-quality metric or a commercial comparison.

Complementary-band reconstruction error in six test sample rates (8/22.05/44.1/48/96/192 kHz): <= 1.2e-16 in the tested sequences. Zero-reduction full-engine test max difference: 0 for the original synthetic buffer.

GCC and Clang smoke tests passed. GCC UBSan tests passed. Stress tests at all six sample rates passed with finite outputs for rapidly changing frequency, modes, HPF, ratio/range and monitor settings. Benchmark output in `engine_regression` is informational and hardware-dependent; no formal realtime CPU claim is made.

## Not verified

- Complete JUCE Windows MSVC/Xcode compile (JUCE SDK download unavailable in this environment)
- pluginval, FL Studio and other DAWs; save/load/restart and real input monitoring
- External `.skelly` file chooser behavior in specific DAW sandbox contexts
- Sustained real-vocal consonant tests, DAW CPU usage, phase perception and clicks under extreme automation

## Compatibility

The existing parameter IDs and manufacturer/plugin IDs remain unchanged. Older host state trees are still accepted. The saved file wrapper remains `SkellyDocument`; external `.skelly` files use that serialized host-state format.
