# Skelly DeEsser V7 — Source UI repair

- Default editor changed from 1152 x 768 to 633 x 422 (55% linear scale).
- Added UI-size migration for V6 sessions; existing parameter values are not reset.
- Added Settings → Window size options: 55, 70, 85, 100% of V6 launch size.
- Fixed preset NEXT button overlapping the preset title.
- Fixed fixed-size section title and star causing cramped/cut text at smaller sizes.
- Rendered a single top-center skull instead of stacking the emblem over the skull already in the frame.
- Increased contrast of controls by darkening the V5 grunge textures under the stage and left column, leaving metal frame details intact.
- Removed a bright fake waveform when input peaks are zero.
- Made readout and button fonts responsive.
- No change to plugin IDs, parameter IDs, DSP engine, or GitHub workflow.
- No new image was generated for this update.

Verify with Windows CI and FL Studio: initial 633x422 sizing, clicking controls, preset switching, resizing, saved/reloaded UI size, HiDPI and audio metering.
