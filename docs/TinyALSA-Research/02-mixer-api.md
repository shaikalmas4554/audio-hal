# Mixer APIs

**Header:** `tinyalsa/mixer.h`
**Purpose:** Control codec settings and routing — volume, mute, capture source, playback path, headphone/speaker enable, mic gain.

TinyALSA doesn't have a separate "mixer" vs "control" header — both live in `mixer.h`. This file covers the **mixer-handle and control-discovery** half (open the mixer, find a control); `03-control-api.md` covers **reading/writing an individual control's value** once you have it. Splitting the doc this way matches the Phase 2 categories even though it's one header.

## 1. Core types

```c
struct mixer;       // opaque handle for a sound card's whole mixer
struct mixer_ctl;    // opaque handle for one control on that mixer

enum mixer_ctl_type {
    MIXER_CTL_TYPE_BOOL,
    MIXER_CTL_TYPE_INT,
    MIXER_CTL_TYPE_ENUM,
    MIXER_CTL_TYPE_BYTE,
    MIXER_CTL_TYPE_IEC958,
    MIXER_CTL_TYPE_INT64,
    MIXER_CTL_TYPE_UNKNOWN,
};
```

## 2. Open / close the mixer

| Function | Responsibility |
|---|---|
| `struct mixer *mixer_open(unsigned int card)` | Opens the mixer for a given sound card index (matches the `card` in `pcm_open`). |
| `void mixer_close(struct mixer *mixer)` | Closes the mixer and frees resources. |
| `int mixer_add_new_ctls(struct mixer *mixer)` | Refreshes the control list — some cards (e.g. HDMI) add controls dynamically after open. |
| `const char *mixer_get_name(const struct mixer *mixer)` | Human-readable name of the sound card. |

## 3. Discovering controls

| Function | Responsibility |
|---|---|
| `unsigned int mixer_get_num_ctls(const struct mixer *mixer)` | Total number of controls on the card. |
| `unsigned int mixer_get_num_ctls_by_name(const struct mixer *mixer, const char *name)` | How many controls share a given name (some controls repeat per-channel). |
| `struct mixer_ctl *mixer_get_ctl(struct mixer *mixer, unsigned int id)` | Look up a control by numeric index. |
| `const struct mixer_ctl *mixer_get_ctl_const(const struct mixer *mixer, unsigned int id)` | Const version of the above. |
| `struct mixer_ctl *mixer_get_ctl_by_name(struct mixer *mixer, const char *name)` | Look up a control by its ALSA name string (e.g. `"Headphone Playback Volume"`, `"Speaker Switch"`) — **this is the one your routing code will use most.** |
| `struct mixer_ctl *mixer_get_ctl_by_name_and_index(struct mixer *mixer, const char *name, unsigned int index)` | Same, but disambiguates when multiple controls share a name (e.g. per-channel controls). |

## 4. Mixer-level events

| Function | Responsibility |
|---|---|
| `int mixer_subscribe_events(struct mixer *mixer, int subscribe)` | Turns on/off notifications for control-value changes (e.g. jack detect, external volume changes). |
| `int mixer_wait_event(struct mixer *mixer, int timeout)` | Blocks until a subscribed event arrives or timeout elapses. |

## Notes for the wrapper

- One `struct mixer *` per sound card — open it once in your HAL and keep it alive for the HAL's lifetime rather than reopening per routing change.
- Confirm exact control name strings on your actual board with `tinymix contents` (WM8960/SGTL5000 ASoC driver names vary) — don't hardcode names from generic examples.
- `mixer_add_new_ctls` matters if you support HDMI or hot-pluggable paths; for a fixed WM8960/SGTL5000 board it's likely a no-op you can skip initially.
