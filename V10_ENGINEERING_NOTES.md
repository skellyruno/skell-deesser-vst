# Skelly DeEsser V10 — DSP efficiency + compact UI refinement

- Same parameter IDs, 633x422 default, artwork, preset state, and 0 reported latency as V9.
- DSP: 12ms smoothing on Ratio and Max Reduction; 10ms smoothing on Stereo Link.
- DSP: avoid redundant biquad coefficient/trigonometric recalculation when the frequency, filter type, and HPF setting are unchanged. Frequency remains interpolated at 20ms as in V9, with bounded coefficient updates during sweeping.
- UI: quieter backdrop/frame opacity, recessed detector/readout rail, event-driven repaint at a 24Hz polling cap (idle frames skipped), readable frequency entry in Hz and kHz, validated threshold entry, contextual tooltips, no repeated preset-caption invalidation. No new image assets.
- Automated checks: existing 7 DSP tests, new automation/CPU-regression test, UI geometry checks, GCC, Clang and UBSan.
- **Limits:** Synthetic signals are no substitute for vocal listening tests. Requires the Windows JUCE/FL Studio build for final verification.
