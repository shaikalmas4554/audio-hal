#ifndef AUDIO_HAL_H
#define AUDIO_HAL_H

#include "audio_device.h"
#include "audio_params.h"
#include "audio_route.h"
#include "audio_stream.h"


// Opaque HAL object.
struct audio_hal;

// HAL state.
enum audio_hal_state {
    AUDIO_HAL_STATE_UNINITIALIZED = 0,
    AUDIO_HAL_STATE_READY,
    AUDIO_HAL_STATE_ACTIVE,
    AUDIO_HAL_STATE_ERROR
};

// Create / destroy HAL.
struct audio_hal *audio_hal_create(void);
void audio_hal_destroy(struct audio_hal *hal);

// HAL lifecycle.
int audio_hal_init(struct audio_hal *hal);
int audio_hal_deinit(struct audio_hal *hal);

// HAL state.
enum audio_hal_state audio_hal_get_state(const struct audio_hal *hal);

// Device information.
int audio_hal_get_device_info(const struct audio_hal *hal, struct audio_device_info *info);

// Playback.
int audio_hal_open_playback(struct audio_hal *hal, const struct audio_params *params);
int audio_hal_write(struct audio_hal *hal, const void *data, unsigned int frames);
int audio_hal_close_playback(struct audio_hal *hal);

// Capture.
int audio_hal_open_capture(struct audio_hal *hal,const struct audio_params *params);
int audio_hal_read(struct audio_hal *hal, void *data, unsigned int frames);
int audio_hal_close_capture(struct audio_hal *hal);

// Standby.
int audio_hal_standby_playback(struct audio_hal *hal);
int audio_hal_standby_capture(struct audio_hal *hal);

// Playback parameters.
int audio_hal_set_playback_params(struct audio_hal *hal,const struct audio_params *params);
int audio_hal_get_playback_params(const struct audio_hal *hal,struct audio_params *params);

// Capture parameters.
int audio_hal_set_capture_params(struct audio_hal *hal,const struct audio_params *params);
int audio_hal_get_capture_params(const struct audio_hal *hal, struct audio_params *params);

// Output routing.
int audio_hal_set_output_device(struct audio_hal *hal, enum audio_output_device device);
enum audio_output_device audio_hal_get_output_device(const struct audio_hal *hal);

// Input routing.
int audio_hal_set_input_device(struct audio_hal *hal, enum audio_input_device device);
enum audio_input_device audio_hal_get_input_device(const struct audio_hal *hal);

// Volume.
int audio_hal_set_volume(struct audio_hal *hal, int volume);
int audio_hal_get_volume(struct audio_hal *hal, int *volume);

// Mute.
int audio_hal_set_mute(struct audio_hal *hal, int mute);
int audio_hal_get_mute(struct audio_hal *hal, int *mute);

#endif /* AUDIO_HAL_H */