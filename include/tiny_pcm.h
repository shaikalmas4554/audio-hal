#ifndef TINY_PCM_H
#define TINY_PCM_H

enum tiny_pcm_direction {
    TINY_PCM_PLAYBACK, // for playback
    TINY_PCM_CAPTURE   // for capture
};

typedef enum tiny_pcm_format_t {
    TINY_PCM_FORMAT_INVALID = -1, 
    TINY_PCM_FORMAT_S16_LE,        // signed 16 bit Little Endian
    TINY_PCM_FORMAT_S32_LE,        // signed 32 bit Little Endian
    TINY_PCM_FORMAT_S8,            // signed 8 bit Little Endian
    TINY_PCM_FORMAT_S24_LE,        // signed 24 bit (32-bit in memory) Little Endian
    TINY_PCM_FORMAT_S24_3LE        // signed 24 bit, Little Endian
}tiny_pcm_format_t;

typedef struct {
    unsigned int channels; 
    unsigned int rate;
    unsigned int period_size;
    unsigned int period_count;
    enum tiny_pcm_format_t format;
} tiny_pcm_config_t;

struct tiny_pcm;

struct tiny_pcm *tiny_pcm_open( unsigned int card, 
    unsigned int device, 
    enum tiny_pcm_direction direction, 
    const tiny_pcm_config_t *config
);

int tiny_pcm_close(struct tiny_pcm *pcm);

int tiny_pcm_is_ready(const struct tiny_pcm *pcm);

int tiny_pcm_write( struct tiny_pcm *pcm, const void *data, unsigned int frame_count);

int tiny_pcm_read( struct tiny_pcm *pcm, void *data, unsigned int frame_count);

const char *tiny_pcm_get_error(const struct tiny_pcm *pcm);

#endif