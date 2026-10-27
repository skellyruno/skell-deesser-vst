# Skelly DeEsser V5 — Generated Artwork Integration

V5 replaces **all seven assets** in `Assets/` with the new gothic/neon-green images supplied in this conversation. No GitHub Actions configuration changes are required.

| File | Role | Packed pixel size |
|---|---|---|
| `frame.png` | Main metal/chain frame with transparent control windows | 1536 × 1024 |
| `skull_emblem.png` | Upper-center glowing skull ring, independent layer | 680 × 680 |
| `stage_bg.png` | Grunge/gridded center texture (opaque) | 1280 × 730 |
| `left_panel_bg.png` | Control column gothic plate (opaque) | 340 × 880 |
| `knob_skull.png` | Rotating frequency knob art | 400 × 400 |
| `fader_cap.png` | Threshold's draggable skull handle; transparent margins trimmed | 420 × 344 |
| `fader_housing.png` | Threshold's spiked recessed metal rail; transparent margins trimmed | 160 × 960 |

The generated originals were resized for reasonable JUCE `BinaryData` build size and fast GUI loading. Transparency is preserved for foreground assets. They remain independent image layers so later artwork revisions can be swapped without modifying DSP.

## UI revisions

- Composites the seven static images into one `juce::Image` once when the editor opens, avoiding seven large scaled texture draws on every repaint.
- Uses a 75% opacity stage texture under the bright metal frame to keep waveform text and the meter legible.
- Places the new frame behind dynamic controls and the fader skull cap **above** the live vertical meter.
- Updates selected-state button gradients for Split, Wide, Audio, and S Chain.
- Renders the real input waveform with a rail exclusion gap to keep it from crossing the fader.
- Keeps sidechain level (`dBFS`) and gain reduction (`dB`) as separate live readouts.
- Preserves parameter IDs, state format, JUCE header generation and the existing GitHub workflow.

## Verification

- `bash tests/run_smoke.sh` passed for detector selectivity, waveform capture and engine transparency/reduction.
- All seven PNG images were decoded by Pillow and validated as valid image assets. The frame and component overlays contain alpha.
- A **design composite** (not a screenshot from compiled JUCE/FL Studio) is provided separately for checking proportions.
- Full JUCE/MSVC and FL Studio runtime validation are still required.

## Build

See `README.md` and `.github/workflows/build.yml`. The JUCE target still calls `juce_generate_juce_header(SkellyDeEsser)` after `juce_add_plugin`.
