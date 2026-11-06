# Skelly DeEsser V8 — Phrase-Aware processing

## User-facing behavior

Select **Phrase** in the left DETECTION section. The processor learns the singer's typical sibilant intensity from completed events *within the current uninterrupted vocal phrase*. It then attenuates only later events whose detector level significantly exceeds the learned baseline. The first detected S in a new phrase is used as a reference rather than being automatically compressed. Its response scales with the singer's recording level, not a fixed absolute threshold.

**Settings → Phrase sensitivity** is a host-automatable 1–12 dB margin, default +5 dB above the learned baseline. In Phrase mode the ordinary Threshold fader adds a **bounded relative sensitivity bias** (`effectiveThreshold = baseline + max(0.5, margin + clamp((Threshold+24)*0.25, -6, 6))`), so the default -24 dB does not disable adaptive compression on quietly recorded vocals, and the existing Ratio and Max Reduction still set the intensity and cap. The processing is always split-band in Phrase mode; Split/Wide sound and parameters are unchanged. A short release/smoothing stage avoids instantaneous on/off gain changes.

## Design

- Reuses SidechainFilter and EnvelopeFollower, and processes with the same ComplementaryBand as Split.
- Broadband input activity detects pauses: 40 ms release on voice energy and 450 ms below -52 dBFS to end a phrase. This is signal-based segmentation, not language recognition.
- A fast filtered-signal envelope (2 ms attack, 16 ms release) finds sibilant events above -56 dBFS and within 26 dB of broadband voice level, and considers an event finished after 26 ms without qualifying high-band energy. Brief impulses under 11 ms are excluded from baseline learning; compression begins only after 6 ms of candidate sibilant energy.
- One observation per completed event, not one per sample. Baseline adaptation is conservative and ignores observations more than 6 dB above normal. Loud outliers are attenuated but do not shift the baseline.
- Sibilance GR is smoothed over approximately 4 ms on attack and 26 ms on release. The detector runs in all modes to enable smooth switching, but attenuates adaptively only in Phrase mode. Blend transitions use the original 8 ms smoothing.
- Zero added audio-buffer/lookahead latency, no allocations or locks in the audio callback, no trained model. Host parameter IDs remain stable: **detectMode still has its original two choices**, with a separate new **phraseAware** Boolean. Sensitivity is **phraseSensitivity**.

## Known trade-offs

- No lookahead means the first S in a phrase has to pass while the mode learns a reference; in phrases with just one prominent sibilant, Phrase mode may leave it untouched. For guaranteed suppression of every loud first S, use Split mode.
- A normal-level reference is acoustically inferred from filtered energy, not explicitly inferred from a singer identity or lyric transcription. Strong bleed/noise, very quiet singing, fast repeated consonants, and musical transients can confuse the voice/phrase segmentation.
- This is a real-time heuristic, not an ML model, and it has not been trained or validated on commercial vocal datasets. Tune Threshold, Frequency, and Sensitivity by ear.
- The learned reference is transient and intentionally not serialized; host state saves the chosen mode and sensitivity only.

## Regression validation

The new `tests/phrase_aware_regression.cpp` uses direct envelope test input to verify: first sibilant left alone, later sibilants eligible for adaptive attenuation, outliers do not corrupt baseline, and a new phrase resets learning. It also feeds synthetic low-frequency vocal energy plus 5.86 kHz sibilant bursts into the unchanged engine architecture, then compares ordinary and loud events against fixed Split mode. `tests/run_smoke.sh` and GitHub Actions include the new test. Full JUCE/MSVC and in-DAW verification still required.
