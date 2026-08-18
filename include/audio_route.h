#ifndef AUDIO_ROUTE_H
#define AUDIO_ROUTE_H


// Opaque route manager.
struct audio_route;

// Output devices.
enum audio_output_device {
    AUDIO_OUTPUT_NONE = 0,
    AUDIO_OUTPUT_SPEAKER,
    AUDIO_OUTPUT_HEADPHONE
};

// Input devices.
enum audio_input_device {
    AUDIO_INPUT_NONE = 0,
    AUDIO_INPUT_MICROPHONE,
    AUDIO_INPUT_LINE_IN
};

// Create / destroy route manager.
struct audio_route *audio_route_create(unsigned int card);
void audio_route_destroy(struct audio_route *route);

// Output routing.
int audio_route_set_output(struct audio_route *route, enum audio_output_device device);
enum audio_output_device audio_route_get_output(const struct audio_route *route);

// Input routing.
int audio_route_set_input(struct audio_route *route, enum audio_input_device device);
enum audio_input_device audio_route_get_input(const struct audio_route *route);

// Volume.
int audio_route_set_volume(struct audio_route *route, int volume);
int audio_route_get_volume(struct audio_route *route,int *volume);

// Mute.
int audio_route_set_mute(struct audio_route *route, int mute);
int audio_route_get_mute(struct audio_route *route, int *mute);

// Apply current route configuration.
int audio_route_apply(struct audio_route *route);

#endif /* AUDIO_ROUTE_H */