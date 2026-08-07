# PCM APIs

**Header:** `tinyalsa/pcm.h`
**Purpose:** Manage playback and recording streams — open/close a PCM device, configure it, read, write, and control stream state.

This is the layer your HAL's stream lifecycle (`open_output_stream`, `write`, `standby`, `close`) will wrap most directly.

## 1. Core types

```c
struct pcm;              // opaque PCM handle
struct pcm_config {
    unsigned int channels;
    unsigned int rate;
    unsigned int period_size;
    unsigned int period_count;
    enum pcm_format format;
    unsigned long start_threshold;
    unsigned long stop_threshold;
    unsigned long silence_threshold;
    unsigned long silence_size;
    unsigned long avail_min;
};

enum pcm_format {
    PCM_FORMAT_S16_LE, PCM_FORMAT_S32_LE, PCM_FORMAT_S8,
    PCM_FORMAT_S24_LE, PCM_FORMAT_S24_3LE,
    PCM_FORMAT_S16_BE, PCM_FORMAT_S24_BE, PCM_FORMAT_S24_3BE, PCM_FORMAT_S32_BE,
    PCM_FORMAT_FLOAT_LE, PCM_FORMAT_FLOAT_BE,
};
```

**Open flags** (bitwise, used in `pcm_open`):
`PCM_OUT`, `PCM_IN`, `PCM_MMAP`, `PCM_NOIRQ`, `PCM_NORESTART`, `PCM_MONOTONIC`, `PCM_NONBLOCK`

**Stream states** (returned via internal state, inspected through helper calls):
`PCM_STATE_OPEN`, `PCM_STATE_SETUP`, `PCM_STATE_PREPARED`, `PCM_STATE_RUNNING`, `PCM_STATE_XRUN`, `PCM_STATE_DRAINING`, `PCM_STATE_SUSPENDED`, `PCM_STATE_DISCONNECTED`

## 2. Open / close

| Function | Responsibility |
|---|---|
| `struct pcm *pcm_open(unsigned int card, unsigned int device, unsigned int flags, const struct pcm_config *config)` | Opens a PCM device by card/device number with the given flags and hardware config. Always check `pcm_is_ready()` after. |
| `struct pcm *pcm_open_by_name(const char *name, unsigned int flags, const struct pcm_config *config)` | Same as above, but by name (e.g. `"hw:0,0"`) instead of numeric card/device. |
| `int pcm_close(struct pcm *pcm)` | Closes the PCM and frees its handle. |
| `int pcm_is_ready(const struct pcm *pcm)` | Returns 1 if the handle/fd is valid, 0 otherwise. Call this right after every `pcm_open`. |

## 3. Configuration & introspection

| Function | Responsibility |
|---|---|
| `int pcm_set_config(struct pcm *pcm, const struct pcm_config *config)` | Re-applies hardware params to an already-open PCM. |
| `const struct pcm_config *pcm_get_config(const struct pcm *pcm)` | Reads back the active config. |
| `unsigned int pcm_get_channels(const struct pcm *pcm)` | Channel count of the open stream. |
| `unsigned int pcm_get_rate(const struct pcm *pcm)` | Sample rate of the open stream. |
| `enum pcm_format pcm_get_format(const struct pcm *pcm)` | Sample format of the open stream. |
| `unsigned int pcm_get_buffer_size(const struct pcm *pcm)` | Total buffer size in frames. |
| `unsigned int pcm_get_subdevice(const struct pcm *pcm)` | Subdevice index in use. |
| `int pcm_get_file_descriptor(const struct pcm *pcm)` | Raw fd, e.g. for `poll()`. |
| `int pcm_get_poll_fd(struct pcm *pcm)` | fd intended specifically for polling readiness. |

## 4. Data transfer

| Function | Responsibility |
|---|---|
| `int pcm_writei(struct pcm *pcm, const void *data, unsigned int frame_count)` | Writes interleaved frames to a playback stream. **Preferred write API.** |
| `int pcm_readi(struct pcm *pcm, void *data, unsigned int frame_count)` | Reads interleaved frames from a capture stream. **Preferred read API.** |
| `int pcm_write(...)` / `int pcm_read(...)` | Deprecated aliases — avoid in new code. |
| `int pcm_mmap_begin(struct pcm *pcm, void **areas, unsigned int *offset, unsigned int *frames)` | Starts a direct mmap transfer window (used with `PCM_MMAP`). |
| `int pcm_mmap_commit(struct pcm *pcm, unsigned int offset, unsigned int frames)` | Commits frames written/read via the mmap window. |
| `int pcm_mmap_avail(struct pcm *pcm)` | Frames available in the mmap buffer. |
| `int pcm_mmap_get_hw_ptr(struct pcm *pcm, unsigned int *hw_ptr, struct timespec *tstamp)` | Current hardware pointer + timestamp for mmap streams. |

## 5. Stream state control

| Function | Responsibility |
|---|---|
| `int pcm_prepare(struct pcm *pcm)` | Moves the stream into the prepared state before starting. |
| `int pcm_start(struct pcm *pcm)` | Explicitly starts the stream (usually implicit on first write/read). |
| `int pcm_stop(struct pcm *pcm)` | Stops the stream immediately. |
| `int pcm_drain(struct pcm *pcm)` | Lets playback finish naturally, then stops. |
| `int pcm_wait(struct pcm *pcm, int timeout)` | Blocks until the stream is ready for I/O or the timeout elapses. |
| `long pcm_get_delay(struct pcm *pcm)` | Current latency in frames. |
| `int pcm_link(struct pcm *pcm1, struct pcm *pcm2)` | Links two PCMs so they prepare/start/stop in sync. |
| `int pcm_unlink(struct pcm *pcm)` | Removes a link. |

## 6. Frame/byte conversion & error handling

| Function | Responsibility |
|---|---|
| `unsigned int pcm_frames_to_bytes(const struct pcm *pcm, unsigned int frames)` | Frame count → byte count for the stream's format/channels. |
| `unsigned int pcm_bytes_to_frames(const struct pcm *pcm, unsigned int bytes)` | Inverse of the above. |
| `unsigned int pcm_format_to_bits(enum pcm_format format)` | Bit width of a given format. |
| `const char *pcm_get_error(const struct pcm *pcm)` | Human-readable error string for the last failure. |
| `int pcm_ioctl(struct pcm *pcm, int code, ...)` | Deprecated escape hatch for raw ALSA ioctls — avoid unless there's no wrapped equivalent. |

## Notes for the wrapper

- Wrap `pcm_open` + `pcm_is_ready` as one call in your HAL — a "successful open" with a dead handle is a common bug source.
- `pcm_writei`/`pcm_readi` are `TINYALSA_WARN_UNUSED_RESULT` — the wrapper must always check the return value (negative = error, and can be short-frame on `PCM_NONBLOCK`).
- Decide early whether your HAL needs the mmap path (`pcm_mmap_*`) or can stay purely on `pcm_writei`/`pcm_readi` — mmap adds real complexity and is usually only worth it for low-latency requirements.
