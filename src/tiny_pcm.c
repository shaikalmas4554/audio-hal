#include "include/tiny_pcm.h"

struct tiny_pcm* tiny_pcm_open( unsigned int card, unsigned int device, enum tiny_pcm_direction direction, const tiny_pcm_config_t *config)
{

}

int tiny_pcm_close(struct tiny_pcm *pcm)
{

}

int tiny_pcm_is_ready(const struct tiny_pcm *pcm)
{

}

int tiny_pcm_write(struct tiny_pcm *pcm, const void *data, unsigned int frame_count)
{

}

int tiny_pcm_read(struct tiny_pcm *pcm, void *data, unsigned int frame_count)
{

}

const char *tiny_pcm_get_error(const struct tiny_pcm *pcm)
{
    
}