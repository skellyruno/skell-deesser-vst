# Skelly DeEsser V3.2 — MSVC compile fixes

- Replace `constexpr juce::Rectangle<int>` layout constants with `inline const`, because the JUCE Rectangle constructor cannot be constant-evaluated in the selected build.
- Use `juce::var::toString()` instead of ambiguous `juce::String(juce::var)` construction.
- Construct editor `SafePointer` within each outer UI click callback and capture it by value in the asynchronous PopupMenu completion callback.
- Preserve `.github/workflows/build.yml` and CMake target/header ordering.

## Validation
Standalone DSP smoke tests may be run with `bash tests/run_smoke.sh` from the project root. Full MSVC + JUCE compilation and FL Studio lifecycle tests remain to be performed in GitHub Actions/on a host machine.
