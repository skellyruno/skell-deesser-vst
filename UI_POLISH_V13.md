# UI polish V13 — compact control and popup behavior

Changes based on the verified SkellCore real-JUCE screenshots:

- Popup menus have consistent dark/green contrast, a small readable font and
  shorter top-level settings categories. Each parameter submenu shows its live
  value, step sizes and reset action, using the original automatable APVTS IDs.
- Filter choices are described and checked; preset import/export/reset are
  grouped and the popup is anchored to the actual JUCE button.
- Frequency and threshold readouts can be single-click edited. Invalid text is
  reverted to the last valid value, and text entry is clamped by the existing
  host range. Double-clicking the actual knob/fader still restores defaults.
- Disabled buttons have distinct low-contrast styling; hover and focus remain
  visible for enabled controls. HPF and filter icons include short labels.
- Frequency knob has fewer tick marks and a brighter live endpoint indicator.
  The long preset name ellipsizes instead of becoming unreadably tiny.
- No new image assets, DSP changes, parameter IDs, latency or preset format
  changes. The SkellCore headless renderer runs on future UI changes.

## Validation needed
- Build/render/inspect using the pinned SkellCore runner and check the popup
  menu placement in the actual host; verify accessibility at 540x360 and HiDPI.
- Linux portable DSP tests and Windows VST3/pluginval remain gating.

## Actual JUCE popup screenshot regression

The SkellCore screenshot job opens real `PopupMenu` windows from the editor's
filter, preset and Settings button handlers on the Windows desktop. It captures
the popup components composited alongside the real JUCE editor (not simulated
menu art), and uploads `popup-filter.png`, `popup-preset.png`,
`popup-settings.png`, and `popup-contact-sheet.png`. The screenshot job now
gates on 24 PNGs plus the existing manifest. All menu windows are dismissed
between captures.
