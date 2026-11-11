# Skelly DeEsser V14 — Windows VST3 only

- All release builds now use JUCE `FORMATS VST3` exclusively. No Standalone target is built or uploaded. The separate SkellCore renderer executable remains development-only.
- Windows CI builds `SkellyDeEsser_VST3` explicitly, packages one VST3 artifact and validates it with pluginval strictness 5.
- Skips two expensive `std::pow()` calls per channel on exact-unity gain samples. No gain curve, parameter IDs, latency, or crossover processing changed.
- Adds deterministic exact-dry output regression for Ratio=1, Range=0 and nontriggering Threshold=0 across 8–192 kHz, three detection modes, opposite-polarity stereo, and rapid detector changes.
- Adds a dynamic detector-filter tooltip so the current filter and HPF selection remain discoverable at the 633×422 default window without adding visual clutter.
- Preserve all existing factory artwork, processor identities, host automation and preset files.

## Verification checklist

- Run GCC/Clang standalone DSP tests including zero-alloc, extreme input and new transparent idle test.
- Run GCC UBSan and Clang ASan/UBSan.
- Build Windows VST3 and pass pluginval strictness 5.
- Build the SkellCore real JUCE renderer and inspect 19 editor scenes plus the real popup captures.
- Manual FL Studio lifecycle, HiDPI, touch/keyboard submenu interaction, real-vocal listening and lower-end Windows CPU profiling remain prerequisites for final commercial release.
