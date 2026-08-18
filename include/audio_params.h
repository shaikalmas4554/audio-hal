#ifndef AUDIO_PARAMS_H
#define AUDIO_PARAMS_H

#include "tiny_pcm.h"

struct audio_params;

//create and destroy
struct audio_params *audio_params_create(void);
void audio_params_destroy(struct audio_params *params);

//config
int audio_params_set_config(struct audio_params *params, const tiny_pcm_config_t *config);
int audio_params_get_config(const struct audio_params *params, tiny_pcm_config_t *config);

//direction
int audio_params_set_direction(struct audio_params *params, enum tiny_pcm_direction direction);
enum tiny_pcm_direction audio_params_get_direction(const struct audio_params *params);

//format
int audio_params_set_format(struct audio_params *params,tiny_pcm_format_t format);
tiny_pcm_format_t audio_params_get_format(const struct audio_params *params);

//sampling rate
int audio_params_set_rate(struct audio_params *params, unsigned int rate);
unsigned int audio_params_get_rate(const struct audio_params *params);

//channels
int audio_params_set_channels(struct audio_params *params, unsigned int channels);
unsigned int audio_params_get_channels(const struct audio_params *params);

//period size
int audio_params_set_period_size(struct audio_params *params,unsigned int period_size);
unsigned int audio_params_get_period_size(const struct audio_params *params);

//period count
int audio_params_set_period_count(struct audio_params *params,unsigned int period_count);
unsigned int audio_params_get_period_count(const struct audio_params *params);

// Validation
int audio_params_validate(const struct audio_params *params);

// Convert HAL parameters to TinyALSA configuration
int audio_params_to_pcm_config(const struct audio_params *params, tiny_pcm_config_t *config);

#endif /* AUDIO_PARAMS_H */