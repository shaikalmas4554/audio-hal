#include <stdlib.h>

#include "../include/audio_params.h"
#include "../include/error.h"
#include "../include/logger.h"


/*
 * Real definition of the opaque "struct audio_params" declared in the
 * header. Nobody outside this file can see these fields - they can only
 * go through the getter/setter functions below.
 */
struct audio_params {
    tiny_pcm_config_t config;
    enum tiny_pcm_direction direction;
};


struct audio_params *audio_params_create(void)
{
    struct audio_params *params;

    params = malloc(sizeof(*params));
    if (params == NULL) {
        LOG_ERROR("failed to allocate audio_params");
        return NULL;
    }

    /* Sensible, safe defaults: 16-bit stereo, 48kHz, playback. */
    params->config.channels = 2;
    params->config.rate = 48000;
    params->config.period_size = 1024;
    params->config.period_count = 2;
    params->config.format = TINY_PCM_FORMAT_S16_LE;
    params->direction = TINY_PCM_PLAYBACK;

    return params;
}

void audio_params_destroy(struct audio_params *params)
{
    /* free(NULL) is legal and does nothing - no need to check. */
    free(params);
}

int audio_params_set_config(struct audio_params *params, const tiny_pcm_config_t *config)
{
    if (params == NULL || config == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    params->config = *config;

    return AUDIO_OK;
}

int audio_params_get_config(const struct audio_params *params, tiny_pcm_config_t *config)
{
    if (params == NULL || config == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    *config = params->config;

    return AUDIO_OK;
}

int audio_params_set_direction(struct audio_params *params, enum tiny_pcm_direction direction)
{
    if (params == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    params->direction = direction;

    return AUDIO_OK;
}

enum tiny_pcm_direction audio_params_get_direction(const struct audio_params *params)
{
    if (params == NULL) {
        LOG_WARN("audio_params_get_direction called with NULL params, defaulting to playback");
        return TINY_PCM_PLAYBACK;
    }

    return params->direction;
}

int audio_params_set_format(struct audio_params *params, tiny_pcm_format_t format)
{
    if (params == NULL || format == TINY_PCM_FORMAT_INVALID)
        return AUDIO_ERROR_INVALID_PARAM;

    params->config.format = format;

    return AUDIO_OK;
}

tiny_pcm_format_t audio_params_get_format(const struct audio_params *params)
{
    if (params == NULL)
        return TINY_PCM_FORMAT_INVALID;

    return params->config.format;
}

int audio_params_set_rate(struct audio_params *params, unsigned int rate)
{
    if (params == NULL || rate == 0)
        return AUDIO_ERROR_INVALID_PARAM;

    params->config.rate = rate;

    return AUDIO_OK;
}

unsigned int audio_params_get_rate(const struct audio_params *params)
{
    if (params == NULL)
        return 0;

    return params->config.rate;
}

int audio_params_set_channels(struct audio_params *params, unsigned int channels)
{
    if (params == NULL || channels == 0)
        return AUDIO_ERROR_INVALID_PARAM;

    params->config.channels = channels;

    return AUDIO_OK;
}

unsigned int audio_params_get_channels(const struct audio_params *params)
{
    if (params == NULL)
        return 0;

    return params->config.channels;
}

int audio_params_set_period_size(struct audio_params *params, unsigned int period_size)
{
    if (params == NULL || period_size == 0)
        return AUDIO_ERROR_INVALID_PARAM;

    params->config.period_size = period_size;

    return AUDIO_OK;
}

unsigned int audio_params_get_period_size(const struct audio_params *params)
{
    if (params == NULL)
        return 0;

    return params->config.period_size;
}

int audio_params_set_period_count(struct audio_params *params, unsigned int period_count)
{
    if (params == NULL || period_count == 0)
        return AUDIO_ERROR_INVALID_PARAM;

    params->config.period_count = period_count;

    return AUDIO_OK;
}

unsigned int audio_params_get_period_count(const struct audio_params *params)
{
    if (params == NULL)
        return 0;

    return params->config.period_count;
}

int audio_params_validate(const struct audio_params *params)
{
    if (params == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    if (params->config.format == TINY_PCM_FORMAT_INVALID) {
        LOG_ERROR("audio_params_validate: invalid format");
        return AUDIO_ERROR_INVALID_PARAM;
    }

    if (params->config.rate == 0) {
        LOG_ERROR("audio_params_validate: rate is 0");
        return AUDIO_ERROR_INVALID_PARAM;
    }

    if (params->config.channels == 0 || params->config.channels > 8) {
        LOG_ERROR("audio_params_validate: channel count %u out of range",
                  params->config.channels);
        return AUDIO_ERROR_INVALID_PARAM;
    }

    if (params->config.period_size == 0) {
        LOG_ERROR("audio_params_validate: period_size is 0");
        return AUDIO_ERROR_INVALID_PARAM;
    }

    if (params->config.period_count == 0) {
        LOG_ERROR("audio_params_validate: period_count is 0");
        return AUDIO_ERROR_INVALID_PARAM;
    }

    return AUDIO_OK;
}

int audio_params_to_pcm_config(const struct audio_params *params, tiny_pcm_config_t *config)
{
    int ret;

    if (params == NULL || config == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    ret = audio_params_validate(params);
    if (ret != AUDIO_OK)
        return ret;

    *config = params->config;

    return AUDIO_OK;
}
