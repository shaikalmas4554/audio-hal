#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/audio_stream.h"
#include "../include/audio_params.h"
#include "../include/tiny_pcm.h"
#include "../include/error.h"
#include "../include/logger.h"


#define ERROR_MSG_LEN 128


/*
 * Real definition of the opaque "struct audio_stream".
 *
 * Note: card/device are kept even while the PCM is closed (standby),
 * so the stream can transparently reopen itself on the next
 * write()/read() call without the caller having to pass them again.
 */
struct audio_stream {
    enum tiny_pcm_direction direction;
    enum audio_stream_state state;

    struct tiny_pcm *pcm;
    struct audio_params *params; /* owns its own copy, not the caller's */

    unsigned int card;
    unsigned int device;

    char last_error[ERROR_MSG_LEN];
};


static void set_error(struct audio_stream *stream, const char *msg)
{
    snprintf(stream->last_error, ERROR_MSG_LEN, "%s", msg);
    LOG_ERROR("audio_stream: %s", msg);
}

/*
 * Opens (or re-opens) the underlying tiny_pcm handle using whatever
 * card/device/params are currently stored on the stream. Used both by
 * audio_stream_open() and by the standby-wake / recover paths.
 */
static int reopen_pcm(struct audio_stream *stream)
{
    tiny_pcm_config_t config;
    int ret;

    ret = audio_params_to_pcm_config(stream->params, &config);
    if (ret != AUDIO_OK) {
        set_error(stream, "invalid stream parameters");
        return ret;
    }

    stream->pcm = tiny_pcm_open(stream->card, stream->device, stream->direction, &config);
    if (stream->pcm == NULL) {
        set_error(stream, "failed to open tiny_pcm");
        return AUDIO_ERROR_OPEN;
    }

    return AUDIO_OK;
}

struct audio_stream *audio_stream_create(enum tiny_pcm_direction direction)
{
    struct audio_stream *stream;

    stream = malloc(sizeof(*stream));
    if (stream == NULL) {
        LOG_ERROR("failed to allocate audio_stream");
        return NULL;
    }

    stream->params = audio_params_create();
    if (stream->params == NULL) {
        free(stream);
        return NULL;
    }

    audio_params_set_direction(stream->params, direction);

    stream->direction = direction;
    stream->state = AUDIO_STREAM_STATE_CLOSED;
    stream->pcm = NULL;
    stream->card = 0;
    stream->device = 0;
    stream->last_error[0] = '\0';

    return stream;
}

void audio_stream_destroy(struct audio_stream *stream)
{
    if (stream == NULL)
        return;

    if (stream->pcm != NULL)
        tiny_pcm_close(stream->pcm);

    audio_params_destroy(stream->params);
    free(stream);
}

int audio_stream_open(struct audio_stream *stream, unsigned int card, unsigned int device, const struct audio_params *params)
{
    tiny_pcm_config_t config;
    int ret;

    if (stream == NULL || params == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    if (stream->state != AUDIO_STREAM_STATE_CLOSED) {
        set_error(stream, "open() called while stream is not closed");
        return AUDIO_ERROR_NOT_READY;
    }

    /* Validate before touching hardware. */
    ret = audio_params_to_pcm_config(params, &config);
    if (ret != AUDIO_OK) {
        set_error(stream, "invalid params passed to open()");
        return ret;
    }

    /* Copy the caller's params into our own owned copy. */
    audio_params_set_config(stream->params, &config);
    audio_params_set_direction(stream->params, stream->direction);

    stream->card = card;
    stream->device = device;

    ret = reopen_pcm(stream);
    if (ret != AUDIO_OK) {
        stream->state = AUDIO_STREAM_STATE_ERROR;
        return ret;
    }

    stream->state = AUDIO_STREAM_STATE_OPEN;

    return AUDIO_OK;
}

int audio_stream_close(struct audio_stream *stream)
{
    if (stream == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    if (stream->pcm != NULL) {
        tiny_pcm_close(stream->pcm);
        stream->pcm = NULL;
    }

    stream->state = AUDIO_STREAM_STATE_CLOSED;

    return AUDIO_OK;
}

int audio_stream_standby(struct audio_stream *stream)
{
    if (stream == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    if (stream->state == AUDIO_STREAM_STATE_CLOSED)
        return AUDIO_ERROR_NOT_READY;

    /* Release the hardware while keeping card/device/params around. */
    if (stream->pcm != NULL) {
        tiny_pcm_close(stream->pcm);
        stream->pcm = NULL;
    }

    stream->state = AUDIO_STREAM_STATE_STANDBY;

    return AUDIO_OK;
}

int audio_stream_write(struct audio_stream *stream, const void *data, unsigned int frames)
{
    int ret;

    if (stream == NULL || data == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    if (stream->direction != TINY_PCM_PLAYBACK)
        return AUDIO_ERROR_NOT_SUPPORTED;

    if (stream->state == AUDIO_STREAM_STATE_STANDBY) {
        ret = reopen_pcm(stream);
        if (ret != AUDIO_OK) {
            stream->state = AUDIO_STREAM_STATE_ERROR;
            return ret;
        }
        stream->state = AUDIO_STREAM_STATE_OPEN;
    }

    if (stream->state == AUDIO_STREAM_STATE_CLOSED || stream->pcm == NULL) {
        set_error(stream, "write() called on a closed stream");
        return AUDIO_ERROR_NOT_READY;
    }

    ret = tiny_pcm_write(stream->pcm, data, frames);
    if (ret < 0) {
        set_error(stream, tiny_pcm_get_error(stream->pcm));
        stream->state = AUDIO_STREAM_STATE_ERROR;
        return ret;
    }

    stream->state = AUDIO_STREAM_STATE_RUNNING;

    return ret;
}

int audio_stream_read(struct audio_stream *stream, void *data, unsigned int frames)
{
    int ret;

    if (stream == NULL || data == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    if (stream->direction != TINY_PCM_CAPTURE)
        return AUDIO_ERROR_NOT_SUPPORTED;

    if (stream->state == AUDIO_STREAM_STATE_STANDBY) {
        ret = reopen_pcm(stream);
        if (ret != AUDIO_OK) {
            stream->state = AUDIO_STREAM_STATE_ERROR;
            return ret;
        }
        stream->state = AUDIO_STREAM_STATE_OPEN;
    }

    if (stream->state == AUDIO_STREAM_STATE_CLOSED || stream->pcm == NULL) {
        set_error(stream, "read() called on a closed stream");
        return AUDIO_ERROR_NOT_READY;
    }

    ret = tiny_pcm_read(stream->pcm, data, frames);
    if (ret < 0) {
        set_error(stream, tiny_pcm_get_error(stream->pcm));
        stream->state = AUDIO_STREAM_STATE_ERROR;
        return ret;
    }

    stream->state = AUDIO_STREAM_STATE_RUNNING;

    return ret;
}

enum audio_stream_state audio_stream_get_state(const struct audio_stream *stream)
{
    if (stream == NULL) {
        LOG_WARN("audio_stream_get_state called with NULL stream");
        return AUDIO_STREAM_STATE_ERROR;
    }

    return stream->state;
}

enum tiny_pcm_direction audio_stream_get_direction(const struct audio_stream *stream)
{
    if (stream == NULL) {
        LOG_WARN("audio_stream_get_direction called with NULL stream, defaulting to playback");
        return TINY_PCM_PLAYBACK;
    }

    return stream->direction;
}

int audio_stream_set_params(struct audio_stream *stream, const struct audio_params *params)
{
    tiny_pcm_config_t config;
    int ret;

    if (stream == NULL || params == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    ret = audio_params_to_pcm_config(params, &config);
    if (ret != AUDIO_OK) {
        set_error(stream, "invalid params passed to set_params()");
        return ret;
    }

    audio_params_set_config(stream->params, &config);

    /* If the stream is already open, apply the new params immediately
     * by closing and reopening the PCM device with them. */
    if (stream->pcm != NULL) {
        tiny_pcm_close(stream->pcm);
        stream->pcm = NULL;

        ret = reopen_pcm(stream);
        if (ret != AUDIO_OK) {
            stream->state = AUDIO_STREAM_STATE_ERROR;
            return ret;
        }

        stream->state = AUDIO_STREAM_STATE_OPEN;
    }

    return AUDIO_OK;
}

int audio_stream_get_params(const struct audio_stream *stream, struct audio_params *params)
{
    tiny_pcm_config_t config;
    int ret;

    if (stream == NULL || params == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    ret = audio_params_get_config(stream->params, &config);
    if (ret != AUDIO_OK)
        return ret;

    audio_params_set_config(params, &config);
    audio_params_set_direction(params, stream->direction);

    return AUDIO_OK;
}

int audio_stream_recover(struct audio_stream *stream)
{
    int ret;

    if (stream == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    LOG_WARN("audio_stream_recover: reopening PCM to recover from error state");

    if (stream->pcm != NULL) {
        tiny_pcm_close(stream->pcm);
        stream->pcm = NULL;
    }

    ret = reopen_pcm(stream);
    if (ret != AUDIO_OK) {
        stream->state = AUDIO_STREAM_STATE_ERROR;
        return ret;
    }

    stream->state = AUDIO_STREAM_STATE_OPEN;

    return AUDIO_OK;
}

const char *audio_stream_get_error(const struct audio_stream *stream)
{
    if (stream == NULL)
        return "invalid stream handle";

    return stream->last_error;
}
