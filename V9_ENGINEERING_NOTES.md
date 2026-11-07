# Skelly DeEsser V9 — safe Phrase-Aware refinement

- Retains APVTS IDs and compatibility with V8 Split/Wide, Phrase switch, and host presets.
- Phrase event hysteresis prevents soft consonants at the trigger boundary from producing repeated training samples; the original start threshold and 26 ms release hold are unchanged.
- Simultaneous host automation of Wide and Phrase now crossfades through a **convex weighted blend**, preventing inverted dry-path weights.
- Added compact Phrase learning/baseline readout to the existing UI without new assets.
- Added `tests/phrase_gate_regression.cpp` to exercise event gating and rapid stereo mode/frequency automation.
- GitHub Actions now checks GCC/Clang/UBSan, UI geometry, and Windows VST3 + Standalone packaging.
- No change to artwork, resizable UI geometry, parameter IDs, or the reported 0 sample latency.

## Limitations

Phrase-Aware is a low-latency *acoustic* per-phrase event tracker, not a lexical ML model; it cannot know what a singer intended. It must observe at least one completed sibilant before computing its reference. Full subjective vocal validation and FL Studio open/close/automation testing are still necessary.
