# TinyALSA Research — Overview & Reading Order

**Goal (Phase 2):** Understand everything TinyALSA provides before wrapping it — read the API, sort it into categories, and end up knowing exactly which functions your Audio HAL will actually need.

**Project context:** This research feeds Phase 2 of the Audio HAL & TinyALSA Bring-Up project — a user-space Audio HAL on an embedded Linux system (i.MX8MM/MP EVK + WM8960/SGTL5000 codec), covering PCM stream lifecycle management and dynamic audio routing.

**Source headers** (tinyalsa/tinyalsa, `include/tinyalsa/`):
- `pcm.h` — PCM interface
- `mixer.h` — Mixer interface (this is also where the "Control APIs" live — TinyALSA doesn't split mixer and control into separate headers, see note in 03)
- `asoundlib.h` — convenience umbrella header that pulls in `pcm.h` + `mixer.h`

## Files in this research set

Read them in this order — each one builds on the last:

| # | File | Category | What you'll know after reading it |
|---|------|----------|-----------------------------------|
| 1 | `01-pcm-api.md` | PCM APIs | How to open/close/configure a PCM device and move audio frames in and out of it |
| 2 | `02-mixer-api.md` | Mixer APIs | How to open the mixer for a card and enumerate/find the controls it exposes |
| 3 | `03-control-api.md` | Control APIs | How to read/write a single mixer control's value safely, by name or id |
| 4 | `04-utility-api.md` | Utility APIs | How to query hardware capabilities, validate formats, and handle errors |

## Why this order

1. **PCM first** — it's the part your HAL's `stream_out`/`stream_in` will wrap directly; the read/write/state-machine model shapes everything else.
2. **Mixer second** — routing and volume live outside the PCM stream, but you need a `struct mixer *` open before you can touch any control.
3. **Control third** — once you can enumerate controls, this is the layer that actually gets/sets a value (volume, mute, mux/route selection).
4. **Utility last** — capability queries and format helpers are things you'll reach for while implementing 1–3, not a standalone workflow — read them last so the earlier APIs give you a reason to care.

## Phase 2 exit criteria

Before moving to Phase 3 (Core Audio HAL Architecture), you should be able to answer:
- Which `pcm_open()` flags and `pcm_config` fields does your codec path actually need?
- Which mixer control names does your WM8960/SGTL5000 ASoC driver expose for routing, volume, and mute (you'll confirm exact names with `tinymix` on the board in the validation step)?
- Which `mixer_ctl_type` values (bool/int/enum) will your routing logic need to branch on?
- Which utility calls (`pcm_is_ready`, `pcm_get_error`, `pcm_params_*`) will your error-handling paths depend on?
