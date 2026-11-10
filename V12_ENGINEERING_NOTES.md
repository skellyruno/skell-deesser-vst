# Skelly DeEsser V12 — first-buffer fidelity, idle UI efficiency, preset safety

- Keeps all existing automation IDs, Split/Wide/Phrase modes, 633x422 window, seven artwork files, and the zero-sample algorithmic latency contract.
- A restored preset's sidechain filter type, HPF, and frequency now take effect from the **first audio sample**, rather than crossfading from the factory detector tap over the first 6 ms. Subsequent mode changes still use the existing 6 ms smoothing.
- The waveform ring writes on its normal audio cadence but exposes a display *revision* that only changes when the visible history changes. After sustained silence, the GUI avoids unnecessary 24Hz waveform repainting.
- The preset loader now rejects XML documents with an empty/forged root and no threshold/frequency parameter state before calling `setStateInformation`; older valid SkellyState-root exports remain supported.
- Added sample-accurate detector restoration tests across 5 rates, all 5 filter modes, and both HPF settings; plus UI waveform activity/idle tests.
- GCC/Clang full standalone DSP tests, clang AddressSanitizer and UndefinedBehaviorSanitizer on hostile scenarios and new regressions, and compact UI geometry checks pass locally.
- Full JUCE/MSVC build, plugin editor rendering, pluginval, long-duration host callback profiling, FL Studio session recall, and subjective testing on dry/wet male/female vocals still require host-level validation. Synthetic DSP stress alone does not certify release quality.

**Design decision:** avoid introducing extra lookahead latency, new dependencies, or altering VST3 parameter identities during a compatibility/stability milestone.

## V12.2 certification checks

- Output consistency across host blocks of 1, 7, 64, 127, 512, 2048, 8192 at 44.1/48/96 kHz, for Split/Wide/Phrase and Audio/Sidechain monitoring; max drift bound 2e-5.
- Visible keyboard-focus ring for compact buttons.
- CI-only pinned pluginval 1.0.4 host validation at strictness 5; VST3 runtime remains dependency-free.
- Local release-mode 64-frame isolated stereo DSP CPU measured ~0.4–0.7% of realtime at 48 kHz and ~0.8–1.1% at 96 kHz on one Linux machine; **not** an FL Studio CPU promise.
