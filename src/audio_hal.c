#include <stdlib.h>

#include "../include/audio_hal.h"
#include "../include/audio_device.h"
#include "../include/audio_params.h"
#include "../include/audio_route.h"
#include "../include/audio_stream.h"
#include "../include/error.h"
#include "../include/logger.h"


/*
 * Default sound card / device numbers.
 *
 * PLACEHOLDER - the audio_hal API gives callers no way to configure
 * which card to use, so this HAL is built for exactly one fixed card.
 * Verify these against your actual board with `aplay -l` / `arecord -l`
 * over the UART console before running on real hardware.
 *
 * imx8mnevk /proc/asound/cards:
 *   0 btscoaudio   - Bluetooth SCO (needs an active SCO link, not a
 *                     normal speaker - do not use for plain playback)
 *   1 imxspdif     - S/PDIF digital out
 *   2 wm8524audio  - analog codec (speaker/headphone) <- playback here
 *   3 micfilaudio  - PDM mic array                    <- capture here
 *
 * NOTE: playback and capture are on DIFFERENT cards on this board
 * (2 vs 3), but audio_device_info only has a single `card` field
 * shared by both. This HAL cannot address both correctly at once
 * until that's fixed at the API level. Set `card` to whichever
 * direction you're actively testing.
 *
 * Confirm the exact device sub-index with `aplay -l` / `arecord -l`
 * (device is usually 0, but verify).
 */
static const struct audio_device_info DEFAULT_DEVICE_INFO = {
    .card = 2,
    .playback_device = 0,
    .capture_device = 0,
    .name = "imx8-audio-hal"
};


/*
 * Real definition of the opaque "struct audio_hal". Owns exactly one
 * audio_device, which in turn owns the mixer/route/streams.
 */
struct audio_hal {
    struct audio_device *device;
    enum audio_hal_state state;
};


static void refresh_state(struct audio_hal *hal)
{
    enum audio_device_state device_state = audio_device_get_state(hal->device);

    switch (device_state) {
    case AUDIO_DEVICE_STATE_ACTIVE:
        hal->state = AUDIO_HAL_STATE_ACTIVE;
        break;

    case AUDIO_DEVICE_STATE_READY:
        hal->state = AUDIO_HAL_STATE_READY;
        break;

    case AUDIO_DEVICE_STATE_ERROR:
        hal->state = AUDIO_HAL_STATE_ERROR;
        break;

    case AUDIO_DEVICE_STATE_CLOSED:
    default:
        hal->state = AUDIO_HAL_STATE_UNINITIALIZED;
        break;
    }
}

/* Small helpers so every public function doesn't repeat the same
 * two-level NULL checks (hal == NULL, hal->device == NULL). */
static struct audio_route *get_route(struct audio_hal *hal)
{
    if (hal == NULL || hal->device == NULL)
        return NULL;

    return audio_device_get_route(hal->device);
}

static struct audio_stream *get_playback_stream(struct audio_hal *hal)
{
    if (hal == NULL || hal->device == NULL)
        return NULL;

    return audio_device_get_playback_stream(hal->device);
}

static struct audio_stream *get_capture_stream(struct audio_hal *hal)
{
    if (hal == NULL || hal->device == NULL)
        return NULL;

    return audio_device_get_capture_stream(hal->device);
}

struct audio_hal *audio_hal_create(void)
{
    struct audio_hal *hal;

    hal = malloc(sizeof(*hal));
    if (hal == NULL) {
        LOG_ERROR("failed to allocate audio_hal");
        return NULL;
    }

    hal->device = NULL;
    hal->state = AUDIO_HAL_STATE_UNINITIALIZED;

    return hal;
}

void audio_hal_destroy(struct audio_hal *hal)
{
    if (hal == NULL)
        return;

    audio_hal_deinit(hal);
    free(hal);
}

int audio_hal_init(struct audio_hal *hal)
{
    int ret;

    if (hal == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    if (hal->state != AUDIO_HAL_STATE_UNINITIALIZED) {
        LOG_ERROR("audio_hal_init: HAL already initialized");
        return AUDIO_ERROR_NOT_READY;
    }

    hal->device = audio_device_create(&DEFAULT_DEVICE_INFO);
    if (hal->device == NULL) {
        hal->state = AUDIO_HAL_STATE_ERROR;
        return AUDIO_ERROR_NO_MEMORY;
    }

    ret = audio_device_open(hal->device);
    if (ret != AUDIO_OK) {
        audio_device_destroy(hal->device);
        hal->device = NULL;
        hal->state = AUDIO_HAL_STATE_ERROR;
        return ret;
    }

    hal->state = AUDIO_HAL_STATE_READY;

    return AUDIO_OK;
}

int audio_hal_deinit(struct audio_hal *hal)
{
    if (hal == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    if (hal->device != NULL) {
        audio_device_destroy(hal->device);
        hal->device = NULL;
    }

    hal->state = AUDIO_HAL_STATE_UNINITIALIZED;

    return AUDIO_OK;
}

enum audio_hal_state audio_hal_get_state(const struct audio_hal *hal)
{
    if (hal == NULL) {
        LOG_WARN("audio_hal_get_state called with NULL hal");
        return AUDIO_HAL_STATE_ERROR;
    }

    return hal->state;
}

int audio_hal_get_device_info(const struct audio_hal *hal, struct audio_device_info *info)
{
    if (hal == NULL || info == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    if (hal->device == NULL)
        return AUDIO_ERROR_NOT_READY;

    return audio_device_get_info(hal->device, info);
}

int audio_hal_open_playback(struct audio_hal *hal, const struct audio_params *params)
{
    int ret;

    if (hal == NULL || params == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    if (hal->device == NULL)
        return AUDIO_ERROR_NOT_READY;

    ret = audio_device_open_playback(hal->device, params);
    if (ret != AUDIO_OK)
        return ret;

    refresh_state(hal);

    return AUDIO_OK;
}

int audio_hal_write(struct audio_hal *hal, const void *data, unsigned int frames)
{
    struct audio_stream *stream = get_playback_stream(hal);

    if (stream == NULL)
        return AUDIO_ERROR_NOT_READY;

    return audio_stream_write(stream, data, frames);
}

int audio_hal_close_playback(struct audio_hal *hal)
{
    int ret;

    if (hal == NULL || hal->device == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    ret = audio_device_close_playback(hal->device);
    if (ret != AUDIO_OK)
        return ret;

    refresh_state(hal);

    return AUDIO_OK;
}

int audio_hal_open_capture(struct audio_hal *hal, const struct audio_params *params)
{
    int ret;

    if (hal == NULL || params == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    if (hal->device == NULL)
        return AUDIO_ERROR_NOT_READY;

    ret = audio_device_open_capture(hal->device, params);
    if (ret != AUDIO_OK)
        return ret;

    refresh_state(hal);

    return AUDIO_OK;
}

int audio_hal_read(struct audio_hal *hal, void *data, unsigned int frames)
{
    struct audio_stream *stream = get_capture_stream(hal);

    if (stream == NULL)
        return AUDIO_ERROR_NOT_READY;

    return audio_stream_read(stream, data, frames);
}

int audio_hal_close_capture(struct audio_hal *hal)
{
    int ret;

    if (hal == NULL || hal->device == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    ret = audio_device_close_capture(hal->device);
    if (ret != AUDIO_OK)
        return ret;

    refresh_state(hal);

    return AUDIO_OK;
}

int audio_hal_standby_playback(struct audio_hal *hal)
{
    struct audio_stream *stream = get_playback_stream(hal);

    if (stream == NULL)
        return AUDIO_ERROR_NOT_READY;

    return audio_stream_standby(stream);
}

int audio_hal_standby_capture(struct audio_hal *hal)
{
    struct audio_stream *stream = get_capture_stream(hal);

    if (stream == NULL)
        return AUDIO_ERROR_NOT_READY;

    return audio_stream_standby(stream);
}

int audio_hal_set_playback_params(struct audio_hal *hal, const struct audio_params *params)
{
    struct audio_stream *stream = get_playback_stream(hal);

    if (stream == NULL || params == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    return audio_stream_set_params(stream, params);
}

int audio_hal_get_playback_params(const struct audio_hal *hal, struct audio_params *params)
{
    struct audio_stream *stream;

    if (hal == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    stream = get_playback_stream((struct audio_hal *)hal);
    if (stream == NULL || params == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    return audio_stream_get_params(stream, params);
}

int audio_hal_set_capture_params(struct audio_hal *hal, const struct audio_params *params)
{
    struct audio_stream *stream = get_capture_stream(hal);

    if (stream == NULL || params == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    return audio_stream_set_params(stream, params);
}

int audio_hal_get_capture_params(const struct audio_hal *hal, struct audio_params *params)
{
    struct audio_stream *stream;

    if (hal == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    stream = get_capture_stream((struct audio_hal *)hal);
    if (stream == NULL || params == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    return audio_stream_get_params(stream, params);
}

int audio_hal_set_output_device(struct audio_hal *hal, enum audio_output_device device)
{
    int ret;
    struct audio_route *route = get_route(hal);

    if (route == NULL)
        return AUDIO_ERROR_NOT_READY;

    ret = audio_route_set_output(route, device);
    if (ret != AUDIO_OK)
        return ret;

    return audio_route_apply(route);
}

enum audio_output_device audio_hal_get_output_device(const struct audio_hal *hal)
{
    struct audio_route *route = get_route((struct audio_hal *)hal);

    return audio_route_get_output(route);
}

int audio_hal_set_input_device(struct audio_hal *hal, enum audio_input_device device)
{
    int ret;
    struct audio_route *route = get_route(hal);

    if (route == NULL)
        return AUDIO_ERROR_NOT_READY;

    ret = audio_route_set_input(route, device);
    if (ret != AUDIO_OK)
        return ret;

    return audio_route_apply(route);
}

enum audio_input_device audio_hal_get_input_device(const struct audio_hal *hal)
{
    struct audio_route *route = get_route((struct audio_hal *)hal);

    return audio_route_get_input(route);
}

int audio_hal_set_volume(struct audio_hal *hal, int volume)
{
    int ret;
    struct audio_route *route = get_route(hal);

    if (route == NULL)
        return AUDIO_ERROR_NOT_READY;

    ret = audio_route_set_volume(route, volume);
    if (ret != AUDIO_OK)
        return ret;

    return audio_route_apply(route);
}

int audio_hal_get_volume(struct audio_hal *hal, int *volume)
{
    struct audio_route *route = get_route(hal);

    if (route == NULL || volume == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    return audio_route_get_volume(route, volume);
}

int audio_hal_set_mute(struct audio_hal *hal, int mute)
{
    int ret;
    struct audio_route *route = get_route(hal);

    if (route == NULL)
        return AUDIO_ERROR_NOT_READY;

    ret = audio_route_set_mute(route, mute);
    if (ret != AUDIO_OK)
        return ret;

    return audio_route_apply(route);
}

int audio_hal_get_mute(struct audio_hal *hal, int *mute)
{
    struct audio_route *route = get_route(hal);

    if (route == NULL || mute == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    return audio_route_get_mute(route, mute);
}
