#include <stdlib.h>

#include "../include/audio_device.h"
#include "../include/audio_params.h"
#include "../include/audio_route.h"
#include "../include/audio_stream.h"
#include "../include/tiny_mixer.h"
#include "../include/error.h"
#include "../include/logger.h"


/*
 * Real definition of the opaque "struct audio_device".
 *
 * The device owns the mixer, the route manager, and both streams -
 * everything above this layer (audio_hal.c) reaches them through the
 * accessor functions below rather than creating its own.
 */
struct audio_device {
    struct audio_device_info info;
    enum audio_device_state state;

    struct tiny_mixer *mixer;
    struct audio_route *route;
    struct audio_stream *playback_stream;
    struct audio_stream *capture_stream;
};


/* Recomputes device->state from the real state of its two streams,
 * so we never have a bookkeeping flag that can drift out of sync. */
static void refresh_state(struct audio_device *device)
{
    enum audio_stream_state playback_state;
    enum audio_stream_state capture_state;

    playback_state = audio_stream_get_state(device->playback_stream);
    capture_state = audio_stream_get_state(device->capture_stream);

    if (playback_state == AUDIO_STREAM_STATE_CLOSED &&
        capture_state == AUDIO_STREAM_STATE_CLOSED) {
        device->state = AUDIO_DEVICE_STATE_READY;
    } else {
        device->state = AUDIO_DEVICE_STATE_ACTIVE;
    }
}

struct audio_device *audio_device_create(const struct audio_device_info *info)
{
    struct audio_device *device;

    if (info == NULL) {
        LOG_ERROR("audio_device_create: info is NULL");
        return NULL;
    }

    device = malloc(sizeof(*device));
    if (device == NULL) {
        LOG_ERROR("failed to allocate audio_device");
        return NULL;
    }

    device->info = *info;
    device->state = AUDIO_DEVICE_STATE_CLOSED;
    device->mixer = NULL;
    device->route = NULL;
    device->playback_stream = NULL;
    device->capture_stream = NULL;

    return device;
}

void audio_device_destroy(struct audio_device *device)
{
    if (device == NULL)
        return;

    audio_device_close(device);
    free(device);
}

int audio_device_open(struct audio_device *device)
{
    if (device == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    if (device->state != AUDIO_DEVICE_STATE_CLOSED) {
        LOG_ERROR("audio_device_open: device is not closed");
        return AUDIO_ERROR_NOT_READY;
    }

    device->mixer = tiny_mixer_open(device->info.card);
    if (device->mixer == NULL) {
        LOG_ERROR("audio_device_open: failed to open mixer for card %u", device->info.card);
        device->state = AUDIO_DEVICE_STATE_ERROR;
        return AUDIO_ERROR_OPEN;
    }

    device->route = audio_route_create(device->info.card);
    if (device->route == NULL) {
        LOG_ERROR("audio_device_open: failed to create route for card %u", device->info.card);
        tiny_mixer_close(device->mixer);
        device->mixer = NULL;
        device->state = AUDIO_DEVICE_STATE_ERROR;
        return AUDIO_ERROR_OPEN;
    }

    device->playback_stream = audio_stream_create(TINY_PCM_PLAYBACK);
    if (device->playback_stream == NULL) {
        LOG_ERROR("audio_device_open: failed to create playback stream");
        audio_route_destroy(device->route);
        device->route = NULL;
        tiny_mixer_close(device->mixer);
        device->mixer = NULL;
        device->state = AUDIO_DEVICE_STATE_ERROR;
        return AUDIO_ERROR_NO_MEMORY;
    }

    device->capture_stream = audio_stream_create(TINY_PCM_CAPTURE);
    if (device->capture_stream == NULL) {
        LOG_ERROR("audio_device_open: failed to create capture stream");
        audio_stream_destroy(device->playback_stream);
        device->playback_stream = NULL;
        audio_route_destroy(device->route);
        device->route = NULL;
        tiny_mixer_close(device->mixer);
        device->mixer = NULL;
        device->state = AUDIO_DEVICE_STATE_ERROR;
        return AUDIO_ERROR_NO_MEMORY;
    }

    device->state = AUDIO_DEVICE_STATE_READY;

    return AUDIO_OK;
}

int audio_device_close(struct audio_device *device)
{
    if (device == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    if (device->playback_stream != NULL) {
        audio_stream_destroy(device->playback_stream);
        device->playback_stream = NULL;
    }

    if (device->capture_stream != NULL) {
        audio_stream_destroy(device->capture_stream);
        device->capture_stream = NULL;
    }

    if (device->route != NULL) {
        audio_route_destroy(device->route);
        device->route = NULL;
    }

    if (device->mixer != NULL) {
        tiny_mixer_close(device->mixer);
        device->mixer = NULL;
    }

    device->state = AUDIO_DEVICE_STATE_CLOSED;

    return AUDIO_OK;
}

int audio_device_get_info(const struct audio_device *device, struct audio_device_info *info)
{
    if (device == NULL || info == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    *info = device->info;

    return AUDIO_OK;
}

enum audio_device_state audio_device_get_state(const struct audio_device *device)
{
    if (device == NULL) {
        LOG_WARN("audio_device_get_state called with NULL device");
        return AUDIO_DEVICE_STATE_ERROR;
    }

    return device->state;
}

struct tiny_mixer *audio_device_get_mixer(struct audio_device *device)
{
    if (device == NULL)
        return NULL;

    return device->mixer;
}

struct audio_route *audio_device_get_route(struct audio_device *device)
{
    if (device == NULL)
        return NULL;

    return device->route;
}

struct audio_stream *audio_device_get_playback_stream(struct audio_device *device)
{
    if (device == NULL)
        return NULL;

    return device->playback_stream;
}

struct audio_stream *audio_device_get_capture_stream(struct audio_device *device)
{
    if (device == NULL)
        return NULL;

    return device->capture_stream;
}

int audio_device_open_playback(struct audio_device *device, const struct audio_params *params)
{
    int ret;

    if (device == NULL || params == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    if (device->state == AUDIO_DEVICE_STATE_CLOSED) {
        LOG_ERROR("audio_device_open_playback: device not opened, call audio_device_open() first");
        return AUDIO_ERROR_NOT_READY;
    }

    ret = audio_stream_open(device->playback_stream, device->info.card,
                             device->info.playback_device, params);
    if (ret != AUDIO_OK)
        return ret;

    refresh_state(device);

    return AUDIO_OK;
}

int audio_device_open_capture(struct audio_device *device, const struct audio_params *params)
{
    int ret;

    if (device == NULL || params == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    if (device->state == AUDIO_DEVICE_STATE_CLOSED) {
        LOG_ERROR("audio_device_open_capture: device not opened, call audio_device_open() first");
        return AUDIO_ERROR_NOT_READY;
    }

    ret = audio_stream_open(device->capture_stream, device->info.card,
                             device->info.capture_device, params);
    if (ret != AUDIO_OK)
        return ret;

    refresh_state(device);

    return AUDIO_OK;
}

int audio_device_close_playback(struct audio_device *device)
{
    int ret;

    if (device == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    ret = audio_stream_close(device->playback_stream);
    if (ret != AUDIO_OK)
        return ret;

    refresh_state(device);

    return AUDIO_OK;
}

int audio_device_close_capture(struct audio_device *device)
{
    int ret;

    if (device == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    ret = audio_stream_close(device->capture_stream);
    if (ret != AUDIO_OK)
        return ret;

    refresh_state(device);

    return AUDIO_OK;
}
