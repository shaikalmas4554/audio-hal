# Utility APIs

**Header:** `tinyalsa/pcm.h` (params + conversion helpers live here, not in a separate utility header)
**Purpose:** Helper functionality — querying hardware capabilities, checking formats, handling errors.

These aren't a distinct TinyALSA header either — they're the parts of `pcm.h` that support the PCM/mixer workflow rather than driving a stream directly. Grouped separately here because they answer a different question: *"what can this hardware do, and did that last call actually work?"*

## 1. Hardware capability queries (`struct pcm_params`)

```c
struct pcm_params;   // opaque — describes what a card/device supports, before opening it

enum pcm_param {
    PCM_PARAM_ACCESS, PCM_PARAM_FORMAT, PCM_PARAM_SUBFORMAT,
    PCM_PARAM_SAMPLE_BITS, PCM_PARAM_FRAME_BITS, PCM_PARAM_CHANNELS,
    PCM_PARAM_RATE, PCM_PARAM_PERIOD_TIME, PCM_PARAM_PERIOD_SIZE,
    PCM_PARAM_PERIOD_BYTES, PCM_PARAM_PERIODS, PCM_PARAM_BUFFER_TIME,
    PCM_PARAM_BUFFER_SIZE, PCM_PARAM_BUFFER_BYTES, PCM_PARAM_TICK_TIME,
};
```

| Function | Responsibility |
|---|---|
| `struct pcm_params *pcm_params_get(unsigned int card, unsigned int device, unsigned int flags)` | Queries the hardware's supported parameter ranges **before opening the PCM** — use this to validate a `pcm_config` will actually work on this board. |
| `void pcm_params_free(struct pcm_params *pcm_params)` | Frees the params struct. |
| `const struct pcm_mask *pcm_params_get_mask(const struct pcm_params *pcm_params, enum pcm_param param)` | Bitmask of supported values for a mask-type param (e.g. which formats are supported). |
| `unsigned int pcm_params_get_min(const struct pcm_params *pcm_params, enum pcm_param param)` | Minimum supported value for an interval-type param (e.g. min rate). |
| `unsigned int pcm_params_get_max(const struct pcm_params *pcm_params, enum pcm_param param)` | Maximum supported value for an interval-type param (e.g. max channels). |
| `int pcm_params_to_string(struct pcm_params *params, char *string, unsigned int size)` | Dumps the full param set to a human-readable string — useful for a one-time capability log during bring-up. |
| `int pcm_params_format_test(struct pcm_params *params, enum pcm_format format)` | Returns 1 if a specific `pcm_format` is supported, 0 otherwise — check before opening with a hardcoded format. |

## 2. Format / size helpers

| Function | Responsibility |
|---|---|
| `unsigned int pcm_format_to_bits(enum pcm_format format)` | Bit width of a given sample format. |
| `unsigned int pcm_frames_to_bytes(const struct pcm *pcm, unsigned int frames)` | Frame count → byte count for an *open* stream (listed here and in PCM doc since it's a conversion helper, not stream I/O). |
| `unsigned int pcm_bytes_to_frames(const struct pcm *pcm, unsigned int bytes)` | Inverse conversion. |

## 3. Error handling

| Function | Responsibility |
|---|---|
| `int pcm_is_ready(const struct pcm *pcm)` | Cheapest sanity check — is the handle usable at all. |
| `const char *pcm_get_error(const struct pcm *pcm)` | Human-readable message for the last error on this PCM handle — your wrapper's error path should always surface this, not just the negative return code. |

## Notes for the wrapper

- Run `pcm_params_get` + `pcm_params_format_test`/`get_min`/`get_max` once during **System Baseline & Hardware Verification** (Phase 1) and log the result — that becomes your source of truth for what `pcm_config` values are legal on the i.MX8MM/MP + codec combo, instead of guessing from datasheets.
- `pcm_params_get` must be called with the PCM **closed** — it queries capability, not live state.
- Every wrapper function that returns a TinyALSA error code should also call `pcm_get_error()` (for PCM-side failures) so HAL-level logs show the real ALSA/codec error string, not just `-EINVAL`.
