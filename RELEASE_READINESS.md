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

## Release evidence checkpoint — 2026-11-13 (development branches only)

This section records *branch-specific* evidence. It does not replace the P0 checkboxes above and does not assert a released binary.

| Gate | Evidence | Decision |
| --- | --- | --- |
| Windows VST3 compile | [Release-gate branch build 43403512201](https://github.com/skellyruno/skell-deesser-vst/actions/runs/43403512201) succeeded | **Branch build passed**; artifact must still be inspected |
| Strictness-5 validation and SHA-256 packaging | [Draft PR #3](https://github.com/skellyruno/skell-deesser-vst/pull/3) changes upload order and checksum packaging; associated Windows job succeeded | **Unmerged**; inspect job steps, ZIP and checksum before treating as release evidence |
| Headless real-JUCE UI | [Diagnostics branch run 43426971156](https://github.com/skellyruno/skell-deesser-vst/actions/runs/43426971156) failed, despite [Windows compile 43426955755](https://github.com/skellyruno/skell-deesser-vst/actions/runs/43426955755) succeeding | **BLOCKED**; inspect the first failed stage and fix the root cause |
| Invalid host-rate DSP regression | [Draft PR #1](https://github.com/skellyruno/skell-deesser-vst/pull/1) adds tests | **Unmerged**; do not assume they pass on main |
| FL Studio acceptance | No witnessed Windows host test or saved/reloaded .flp evidence | **UNVERIFIED**; cannot release |
| UI polish / accessibility | Source includes compact sizing and renderer scenes; no passing current-branch real-editor screenshots verified here | **UNVERIFIED** for visual clipping, controls, menus and state retention |

### P0 manual acceptance record (fill with evidence, not guesses)

Run this on a clean Windows installation with FL Studio after merging a *green* release branch. Record the date, Windows version, FL Studio version, plugin binary SHA-256, sample rate, buffer size, and video/screenshot or log evidence for each result.

1. **Install and scan:** verify one VST3 in the expected Windows VST3 directory, clean scan, one unique plugin entry, correct plugin name and version. Do not overwrite another plugin's installation.
2. **Host lifecycle:** open/close the editor 10 times, remove/reinsert plugin, reopen a saved project, switch A/B, import/export a .skelly preset and verify exact recalled values. Verify old projects still load.
3. **Audio:** bypass transparency, silence/denormals, S/SH/CH/T sibilants, Phrase vs Split vs Wide, extreme automation, zero added reported latency, no NaN/Inf or audible pops; compare rendered bounce with live output.
4. **Visual QA:** inspect **four representative scenes only** for the release review: default 633×422, Settings/preset menus, an alternate supported UI state (do not invent a new theme), and minimum 540×360. Check 100%, 150% and 200% Windows display scaling; record clipping, overlaps, text contrast, focus and button states. The automated renderer may produce more diagnostic captures, but the human review set is four.
5. **Performance:** monitor sustained 10/25/50 instances and record worst audio callback time, peak CPU, memory and glitches at 64/256/1024 sample buffers; prioritize modest Ryzen/Vega-class Windows machines.
6. **Distribution:** extract the ZIP, independently verify its SHA-256 against the companion checksum, confirm the actual x64 VST3 binary, run pluginval strictness 5 and the VST3 validator where available. Do not call a compile-only ZIP a validated release.

**Release rule:** any missing P0 evidence, crashing UI renderer, failing validator, or failed host save/restore means **NO-GO**. Keep PRs draft/unmerged until their relevant checks are green and reviewed. Paid/free/trial entitlement and public checkout remain separate decisions; no release or sale is implied by this checklist.
