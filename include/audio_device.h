#ifndef AUDIO_DEVICE_H
#define AUDIO_DEVICE_H

#include "audio_params.h"
#include "audio_route.h"
#include "audio_stream.h"
#include "tiny_mixer.h"


// Forward declarations.
struct audio_device;

// Device state.
enum audio_device_state {
    AUDIO_DEVICE_STATE_CLOSED = 0,
    AUDIO_DEVICE_STATE_READY,
    AUDIO_DEVICE_STATE_ACTIVE,
    AUDIO_DEVICE_STATE_ERROR
};

// Device information.
struct audio_device_info {
    unsigned int card;
    unsigned int playback_device;
    unsigned int capture_device;
    const char *name;
};

// Create / destroy.
struct audio_device *audio_device_create(const struct audio_device_info *info);

void audio_device_destroy(struct audio_device *device);

// Device lifecycle.
int audio_device_open(struct audio_device *device);
int audio_device_close(struct audio_device *device);

// Device information.
int audio_device_get_info(const struct audio_device *device,struct audio_device_info *info);


// Device state.
enum audio_device_state audio_device_get_state(const struct audio_device *device);

// Mixer access.
struct tiny_mixer *audio_device_get_mixer(struct audio_device *device);

// Route access.
struct audio_route *audio_device_get_route(struct audio_device *device);

// Playback stream.
struct audio_stream *audio_device_get_playback_stream(struct audio_device *device);

// Capture stream.
struct audio_stream *audio_device_get_capture_stream(struct audio_device *device);

// Open playback.
int audio_device_open_playback(struct audio_device *device,const struct audio_params *params);

// Open capture.
int audio_device_open_capture(struct audio_device *device,const struct audio_params *params);

// Close playback/capture.
int audio_device_close_playback(struct audio_device *device);
int audio_device_close_capture(struct audio_device *device);

#endif /* AUDIO_DEVICE_H */