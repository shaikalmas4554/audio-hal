#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include <stddef.h>

#include "audio_params.h"
#include "tiny_pcm.h"


// Opaque stream object.
struct audio_stream;

// Stream state.
enum audio_stream_state {
    AUDIO_STREAM_STATE_CLOSED = 0,
    AUDIO_STREAM_STATE_OPEN,
    AUDIO_STREAM_STATE_ACTIVE,
    AUDIO_STREAM_STATE_RUNNING,
    AUDIO_STREAM_STATE_STANDBY,
    AUDIO_STREAM_STATE_PAUSED,
    AUDIO_STREAM_STATE_ERROR
};

// Stream creation / destruction.
struct audio_stream *audio_stream_create(enum tiny_pcm_direction direction);
void audio_stream_destroy(struct audio_stream *stream);

// PCM lifecycle.
int audio_stream_open(struct audio_stream *stream, unsigned int card, unsigned int device, const struct audio_params *params);
int audio_stream_close(struct audio_stream *stream);
int audio_stream_standby(struct audio_stream *stream);

// PCM data operations.
int audio_stream_write(struct audio_stream *stream, const void *data, unsigned int frames);
int audio_stream_read(struct audio_stream *stream, void *data, unsigned int frames);

// Stream state.
enum audio_stream_state audio_stream_get_state(const struct audio_stream *stream);

// Stream direction.
enum tiny_pcm_direction audio_stream_get_direction(const struct audio_stream *stream);

// Stream parameters.
int audio_stream_set_params(struct audio_stream *stream,const struct audio_params *params);
int audio_stream_get_params(const struct audio_stream *stream, struct audio_params *params);

// PCM recovery.
int audio_stream_recover(struct audio_stream *stream);

// Error information.
const char *audio_stream_get_error(const struct audio_stream *stream);


#endif /* AUDIO_STREAM_H */