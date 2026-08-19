#include <stdlib.h>

#include "../include/tiny_pcm.h"
#include <tinyalsa/asoundlib.h>

struct tiny_pcm {
    struct pcm *handle;
};

static unsigned int to_tinyalsa_direction(enum tiny_pcm_direction direction)
{
    switch (direction) {
    case TINY_PCM_CAPTURE:
        return PCM_IN;

    case TINY_PCM_PLAYBACK:
        return PCM_OUT;

    default:
        return 0;
    }
}

static enum pcm_format to_tinyalsa_format(enum tiny_pcm_format_t format)
{
    switch (format) {
    case TINY_PCM_FORMAT_INVALID: return PCM_FORMAT_INVALID;

    case TINY_PCM_FORMAT_S16_LE: return PCM_FORMAT_S16_LE;

    case TINY_PCM_FORMAT_S32_LE: return PCM_FORMAT_S32_LE;

    case TINY_PCM_FORMAT_S8: return PCM_FORMAT_S8;

    case TINY_PCM_FORMAT_S24_LE: return PCM_FORMAT_S24_LE;

    case TINY_PCM_FORMAT_S24_3LE: return PCM_FORMAT_S24_3LE;

    default:
        return PCM_FORMAT_INVALID;
    }
}

static struct pcm_config to_tinyalsa_config(const tiny_pcm_config_t *config)
{
    struct pcm_config tiny_config = {0};

    tiny_config.channels = config->channels;
    tiny_config.rate = config->rate;
    tiny_config.period_size = config->period_size;
    tiny_config.period_count = config->period_count;
    tiny_config.format = to_tinyalsa_format(config->format);

    return tiny_config;
}

struct tiny_pcm *tiny_pcm_open( unsigned int card, unsigned int device, enum tiny_pcm_direction direction, const tiny_pcm_config_t *config)
{
    struct tiny_pcm *tiny_pcm;
    struct pcm_config tiny_config;

    if (config == NULL)
        return NULL;

    tiny_pcm = malloc(sizeof(*tiny_pcm));
    if (tiny_pcm == NULL)
        return NULL;

    tiny_config = to_tinyalsa_config(config);

    tiny_pcm->handle = pcm_open(
        card,
        device,
        to_tinyalsa_direction(direction),
        &tiny_config
    );

    if (tiny_pcm->handle == NULL) {
        free(tiny_pcm);
        return NULL;
    }

    /* tinyalsa's pcm_open() virtually never returns NULL, even when the
     * requested card/device/config could not actually be negotiated
     * with the hardware (e.g. wrong card, unsupported rate/format).
     * It returns a valid-looking but "not ready" handle instead, whose
     * internal buffers were never set up. Writing/reading through such
     * a handle can crash inside tinyalsa (NULL mmap access) rather than
     * returning a clean error - so is_ready() must be checked here. */
    if (!pcm_is_ready(tiny_pcm->handle)) {
        pcm_close(tiny_pcm->handle);
        free(tiny_pcm);
        return NULL;
    }

    return tiny_pcm;
}

int tiny_pcm_close(struct tiny_pcm *pcm)
{
    int ret;

    if (pcm == NULL)
        return -1;

    ret = pcm_close(pcm->handle);
    free(pcm);

    return ret;
}

int tiny_pcm_is_ready(const struct tiny_pcm *pcm)
{
    if (pcm == NULL)
        return 0;

    return pcm_is_ready(pcm->handle);
}

int tiny_pcm_write(
    struct tiny_pcm *pcm,
    const void *data,
    unsigned int frame_count)
{
    if (pcm == NULL)
        return -1;

    return pcm_writei(pcm->handle, data, frame_count);
}

int tiny_pcm_read(
    struct tiny_pcm *pcm,
    void *data,
    unsigned int frame_count)
{
    if (pcm == NULL)
        return -1;

    return pcm_readi(pcm->handle, data, frame_count);
}

const char *tiny_pcm_get_error(const struct tiny_pcm *pcm)
{
    if (pcm == NULL)
        return "invalid pcm handle";

    return pcm_get_error(pcm->handle);
}