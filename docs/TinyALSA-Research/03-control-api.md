# Control APIs

**Header:** `tinyalsa/mixer.h` (same header as Mixer APIs — see note below)
**Purpose:** Access individual mixer controls by name or identifier and modify them safely.

> **Header note:** TinyALSA doesn't ship a separate `control.h`. Once you have a `struct mixer_ctl *` from `02-mixer-api.md` (via `mixer_get_ctl_by_name` etc.), everything below operates on it. Treat this file as "what you do with a control after you've found it."

## 1. Inspecting a control before touching it

| Function | Responsibility |
|---|---|
| `unsigned int mixer_ctl_get_id(const struct mixer_ctl *ctl)` | Numeric id of the control. |
| `const char *mixer_ctl_get_name(const struct mixer_ctl *ctl)` | Name string of the control. |
| `enum mixer_ctl_type mixer_ctl_get_type(const struct mixer_ctl *ctl)` | Returns BOOL / INT / ENUM / BYTE / IEC958 / INT64 — **check this before deciding get/set path.** |
| `const char *mixer_ctl_get_type_string(const struct mixer_ctl *ctl)` | Same, as a printable string. |
| `unsigned int mixer_ctl_get_num_values(const struct mixer_ctl *ctl)` | How many values the control holds (e.g. 2 for a stereo volume control). |
| `unsigned int mixer_ctl_get_num_enums(const struct mixer_ctl *ctl)` | Number of choices, for enum-type controls (e.g. input source select). |
| `const char *mixer_ctl_get_enum_string(struct mixer_ctl *ctl, unsigned int enum_id)` | Human-readable label of one enum choice. |
| `void mixer_ctl_update(struct mixer_ctl *ctl)` | Refreshes a control whose metadata can change at runtime (e.g. HDMI EDID-driven controls). |
| `int mixer_ctl_is_access_tlv_rw(const struct mixer_ctl *ctl)` | Whether the control supports TLV (Transport-Level Value) read/write — relevant for byte-blob controls like EQ curves. |

## 2. Reading / writing values

| Function | Responsibility |
|---|---|
| `int mixer_ctl_get_value(const struct mixer_ctl *ctl, unsigned int id)` | Reads the raw integer value at index `id` (e.g. index 0 = left channel). |
| `int mixer_ctl_set_value(struct mixer_ctl *ctl, unsigned int id, int value)` | Writes a raw integer value at index `id`. |
| `int mixer_ctl_get_percent(const struct mixer_ctl *ctl, unsigned int id)` | Reads a volume-type control as a 0–100 percentage instead of raw range. |
| `int mixer_ctl_set_percent(struct mixer_ctl *ctl, unsigned int id, int percent)` | Writes a volume-type control as a percentage — usually nicer for a HAL's `setVolume()` than raw values. |
| `int mixer_ctl_get_array(const struct mixer_ctl *ctl, void *array, size_t count)` | Reads a multi-value/byte-blob control in one call. |
| `int mixer_ctl_set_array(struct mixer_ctl *ctl, const void *array, size_t count)` | Writes a multi-value/byte-blob control in one call. |
| `int mixer_ctl_set_enum_by_string(struct mixer_ctl *ctl, const char *string)` | Sets an enum-type control by its label instead of numeric index — **this is the routing/input-source-select call.** |

## 3. Range queries (for INT-type controls)

| Function | Responsibility |
|---|---|
| `int mixer_ctl_get_range_min(const struct mixer_ctl *ctl)` | Minimum legal raw value. |
| `int mixer_ctl_get_range_max(const struct mixer_ctl *ctl)` | Maximum legal raw value. |

## Mapping to HAL responsibilities

| HAL need | Control type | Calls to use |
|---|---|---|
| Volume (speaker/headphone/mic gain) | INT | `mixer_ctl_get/set_percent` or `get/set_value` + range queries |
| Mute | BOOL | `mixer_ctl_get/set_value` (0/1) |
| Playback path / capture source select | ENUM | `mixer_ctl_get_num_enums` + `mixer_ctl_get_enum_string` to discover choices, `mixer_ctl_set_enum_by_string` to switch |
| Headphone/speaker enable (jack switch) | BOOL or ENUM | depends on the codec driver — confirm with `tinymix` |

## Notes for the wrapper

- Always call `mixer_ctl_get_type()` before assuming get/set semantics — treating an ENUM control as an INT (or vice versa) is a common wrapper bug.
- For stereo controls, `mixer_ctl_get_num_values()` is usually 2 — loop over `id` 0 and 1 rather than assuming mono.
- Prefer `set_enum_by_string` over guessing numeric enum indices — indices aren't guaranteed stable across kernel/driver versions, but the label strings are.
