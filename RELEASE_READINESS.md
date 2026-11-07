# Skelly DeEsser — release readiness checklist (V11, 2026-11-07)

Status legend: [x] verified by the named test; [ ] not yet verified. **Do not call the plugin production-ready until all P0 gates pass.**

## P0 — Must pass before public release

- [x] JUCE MSVC V10 VST3/Standalone build passed in GitHub Actions (run 42403563523). V11 must separately be verified.
- [x] Current standalone DSP unit/regression tests (GCC/Clang).
- [x] 11-sample-rate synthetic hostile DSP stress and zero-reduction transparency, plus ASan/UBSan for the tested path.
- [x] Zero heap allocations during 700 hostile blocks in isolated DeEsserEngine.
- [x] Defensive handling of NaN/Inf audio/parameters and startup preset-state correctness.
- [ ] **V11 Windows VST3 + Standalone build** and artifacts inspected (new CI run required).
- [ ] **FL Studio lifecycle**: scan/install plugin, open/close GUI repeatedly, delete instance, save/reload project, undo/redo, plugin bridge, repeat at 100/125/150/200% Windows display scale.
- [ ] **Host automation**: all parameter IDs automate smoothly, no drops/clicks; reopen an old V6–V10 .flp and verify same settings/sound.
- [ ] **State/presets**: A/B copy+switch; .skelly import/export, corrupted/foreign/oversized preset handling, saved resized window size, deterministic project recall.
- [ ] **Plugin validator**: pluginval strictness 5+, VST3 validator, JUCE `AudioPluginHost` smoke test (where available).
- [ ] **Realtime profiling in a DAW**: sustained use of 10/25/50+ instances, heavy project load, 64–2048 block sizes, multiple sound cards, no audio glitches; quantify worst-case callback time and allocations.

## P1 — Sound-quality release criteria

- [ ] Build a licensed vocal test corpus (male/female, high/low register, whispers, breaths, plosives, S/SH/CH/T, dry/processed stems, quiet/hot), plus explicit permissions.
- [ ] Compare Fixed Split/Wide, Phrase, and leading de-essers on normal-vs-outlier consonants with level-matched blind A/B; measure consonant recall/false reductions.
- [ ] Test attack/release sensitivity, stereo stability and imaging, aliasing/phase coloration of split-band mid extraction, distortion at high GR, and threshold automation.
- [ ] Implement stronger sibilant vs breath/hi-hat discrimination and robust phrase resets under music/noise, measured against annotated clips.
- [ ] Test sample rates 44.1/48/88.2/96/192 kHz on real DAWs and verify reported zero latency and offline/bounce parity.

## P1 — Finish the 633 × 422 interface

- [x] Vector button icons; compact resizable 3:2 UI; numeric input validation; readable Phrase status and distinct detector meter labels.
- [ ] Screenshot and manual click/keyboard/scroll tests at minimum 540×360, default 633×422 and full 1536×1024 on 100/150/200% display scale.
- [ ] Improve accessible focus order, keyboard navigation, screen-reader names, tooltips and high-contrast toggle indications.
- [ ] Make all advanced DSP settings easy to discover and edit beyond incremental submenus; host automation visible on UI.
- [ ] Fix any render clipping/overlap observed in real FL Studio screenshots, not conceptual mockups.

## P2 — Distribution & support

- [ ] Final installer/ZIP layout, checksums, plugin version numbers, basic manual, demo audio and changelog.
- [ ] Build Windows and macOS if shipping on both platforms; test clean-machine installation, AV false positives and signed artifacts as appropriate.
- [ ] Decide paid/free/trial entitlement flow and validate failure/offline grace paths only after audio quality and host stability gates pass.
- [ ] Version/release notes, support contact, bug report templates, regression fixtures, and reproducible release CI.