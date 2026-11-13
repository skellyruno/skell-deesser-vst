# Skelly DeEsser — Windows VST3 release evidence record

Copy this template into the private release review for each candidate build. **Do not mark PASS without an artifact or host observation.** This is a record template, not evidence of a passing build.

## Candidate identity

- Release version: NOT VERIFIED
- Source commit (full SHA): NOT VERIFIED
- GitHub Actions run URL: NOT VERIFIED
- VST3 artifact filename: NOT VERIFIED
- ZIP SHA-256 (64 hexadecimal characters): NOT VERIFIED
- Extracted VST3 x64 binary SHA-256: NOT VERIFIED
- Windows / FL Studio versions: NOT VERIFIED
- Sample rate / audio buffer: NOT VERIFIED
- Tester and test date: NOT VERIFIED

## Release gates

| Gate | Status | Evidence link / local record |
| --- | --- | --- |
| CI build for **this exact commit** | UNVERIFIED | |
| pluginval strictness 5 for **this exact binary** | UNVERIFIED | |
| ZIP checksum recomputed independently | UNVERIFIED | |
| Extracted PE32+ x64 binary and VST3 bundle layout inspected | UNVERIFIED | |
| Clean Windows installation and FL Studio scan | UNVERIFIED | |
| Open/close/delete/reinsert editor; no crash | UNVERIFIED | |
| Save/reload .flp; automation and preset recall | UNVERIFIED | |
| UI default, Settings/presets, alternate supported state, compact | UNVERIFIED | |
| UI 100/150/200% display scaling, controls and menu interaction | UNVERIFIED | |
| Audio quality, silence, clipping, offline bounce, latency | UNVERIFIED | |
| CPU and memory measurements on modest Windows hardware | UNVERIFIED | |
| License, third-party attribution, distribution terms reviewed | UNVERIFIED | |

## UI defect record

For each observed issue, record: scene, dimensions, Windows scale, reproduction steps, expected vs actual result, screenshot/log path, source commit, fix PR, and retest outcome. Never substitute a conceptual mockup for a captured real editor.

## Decision

**NO-GO** until all P0 rows are supported by reproducible evidence and the headless renderer crash is resolved. Do not publish installers, checkout links, or trial licensing changes from this template.
