# Skelly DeEsser — LOCAL REAL JUCE SCREENSHOTS 🩻

This repository participates in SkellCore's four-scene local UI QA. **Don't generate a replacement UI image or present an audio waveform as a screenshot.** Capture the actual C++ `PluginEditor` instantiated with the real processor and component hierarchy.

- CMake option: `-DSKELLY_DEESSER_BUILD_UI_RENDERER=ON`
- Development renderer target: `SkellyDeEsserUIRenderer`
- Default: exactly **four** actual states: `compact_split, compact_phrase, compact_preset_b, compact_frequency_high`
- `--full`: opt-in previous comprehensive UI regression
- Special case: selected preset in Quick4; actual opened popups available only in --full
- Current verification: **local native compilation pending**. A committed renderer is not proof of a successful build on the latest revision.

Common entrypoint: `skellcore-vst-template/tools/quick4_projects.py --plugin skell-deesser-vst --project /path/to/skell-deesser-vst --juce /path/to/JUCE --skellcore /path/to/skellcore-vst-template --output /tmp/skell-deesser-vst-quick4`.

On restricted machines, source pinned dependencies must already exist; never quietly substitute different live plugin code or claim the screenshot is current if source compiling fails.

GitHub Actions is not needed for ordinary screenshots; any existing screenshot-only workflow is **manual-only**. Maintain VST3, DSP/latency/CPU, Windows, and FL Studio validation separately.
