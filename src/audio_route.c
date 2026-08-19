#include <stdlib.h>

#include "../include/audio_route.h"
#include "../include/tiny_mixer.h"
#include "../include/error.h"
#include "../include/logger.h"


/*
 * Mixer control names - PLACEHOLDERS.
 *
 * These are the ALSA control names that get looked up via
 * tiny_mixer_get_ctl_by_name(). WM8960 and SGTL5000 ASoC drivers use
 * different names, and both differ from these examples. Before running
 * on real hardware, run `tinymix contents` over the UART console and
 * replace these with the actual control names for your codec.
 */
#define CTL_SPEAKER_SWITCH    "Speaker Switch"
#define CTL_HEADPHONE_SWITCH  "Headphone Switch"
#define CTL_MIC_SWITCH        "Mic Switch"
#define CTL_LINEIN_SWITCH     "Line In Switch"
#define CTL_PLAYBACK_VOLUME   "Headphone Playback Volume"
#define CTL_PLAYBACK_MUTE     "Headphone Playback Switch"

#define VOLUME_MIN 0
#define VOLUME_MAX 100
#define VOLUME_DEFAULT 70


/*
 * Real definition of the opaque "struct audio_route". Holds the desired
 * routing state in memory - nothing here touches hardware until
 * audio_route_apply() is called.
 */
struct audio_route {
    unsigned int card;
    struct tiny_mixer *mixer;

    enum audio_output_device output;
    enum audio_input_device input;
    int volume; /* 0-100 */
    int mute;   /* 0 = unmuted, 1 = muted */
};


/*
 * Best-effort helper: enable/disable a boolean mixer switch by name.
 * Logs a warning instead of failing if the control doesn't exist on
 * this codec - not every board has every control.
 */
static void apply_switch(struct tiny_mixer *mixer, const char *name, int enable)
{
    struct tiny_mixer_ctl *ctl;

    ctl = tiny_mixer_get_ctl_by_name(mixer, name);
    if (ctl == NULL) {
        LOG_WARN("mixer control '%s' not found, skipping", name);
        return;
    }

    if (tiny_mixer_ctl_set_value(ctl, 0, enable) != 0)
        LOG_WARN("failed to set mixer control '%s'", name);
}

static int is_valid_output_device(enum audio_output_device device)
{
    return device == AUDIO_OUTPUT_NONE ||
           device == AUDIO_OUTPUT_SPEAKER ||
           device == AUDIO_OUTPUT_HEADPHONE;
}

static int is_valid_input_device(enum audio_input_device device)
{
    return device == AUDIO_INPUT_NONE ||
           device == AUDIO_INPUT_MICROPHONE ||
           device == AUDIO_INPUT_LINE_IN;
}

struct audio_route *audio_route_create(unsigned int card)
{
    struct audio_route *route;

    route = malloc(sizeof(*route));
    if (route == NULL) {
        LOG_ERROR("failed to allocate audio_route");
        return NULL;
    }

    route->mixer = tiny_mixer_open(card);
    if (route->mixer == NULL) {
        LOG_ERROR("audio_route_create: failed to open mixer for card %u", card);
        free(route);
        return NULL;
    }

    route->card = card;
    route->output = AUDIO_OUTPUT_NONE;
    route->input = AUDIO_INPUT_NONE;
    route->volume = VOLUME_DEFAULT;
    route->mute = 0;

    return route;
}

void audio_route_destroy(struct audio_route *route)
{
    if (route == NULL)
        return;

    tiny_mixer_close(route->mixer);
    free(route);
}

int audio_route_set_output(struct audio_route *route, enum audio_output_device device)
{
    if (route == NULL || !is_valid_output_device(device))
        return AUDIO_ERROR_INVALID_PARAM;

    route->output = device;

    return AUDIO_OK;
}

enum audio_output_device audio_route_get_output(const struct audio_route *route)
{
    if (route == NULL)
        return AUDIO_OUTPUT_NONE;

    return route->output;
}

int audio_route_set_input(struct audio_route *route, enum audio_input_device device)
{
    if (route == NULL || !is_valid_input_device(device))
        return AUDIO_ERROR_INVALID_PARAM;

    route->input = device;

    return AUDIO_OK;
}

enum audio_input_device audio_route_get_input(const struct audio_route *route)
{
    if (route == NULL)
        return AUDIO_INPUT_NONE;

    return route->input;
}

int audio_route_set_volume(struct audio_route *route, int volume)
{
    if (route == NULL || volume < VOLUME_MIN || volume > VOLUME_MAX)
        return AUDIO_ERROR_INVALID_PARAM;

    route->volume = volume;

    return AUDIO_OK;
}

int audio_route_get_volume(struct audio_route *route, int *volume)
{
    if (route == NULL || volume == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    *volume = route->volume;

    return AUDIO_OK;
}

int audio_route_set_mute(struct audio_route *route, int mute)
{
    if (route == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    route->mute = mute ? 1 : 0;

    return AUDIO_OK;
}

int audio_route_get_mute(struct audio_route *route, int *mute)
{
    if (route == NULL || mute == NULL)
        return AUDIO_ERROR_INVALID_PARAM;

    *mute = route->mute;

    return AUDIO_OK;
}

int audio_route_apply(struct audio_route *route)
{
    struct tiny_mixer_ctl *volume_ctl;

    if (route == NULL || route->mixer == NULL)
        return AUDIO_ERROR_NOT_READY;

    /* Output routing: enable only the currently-selected path. */
    apply_switch(route->mixer, CTL_SPEAKER_SWITCH, route->output == AUDIO_OUTPUT_SPEAKER);
    apply_switch(route->mixer, CTL_HEADPHONE_SWITCH, route->output == AUDIO_OUTPUT_HEADPHONE);

    /* Input routing: enable only the currently-selected source. */
    apply_switch(route->mixer, CTL_MIC_SWITCH, route->input == AUDIO_INPUT_MICROPHONE);
    apply_switch(route->mixer, CTL_LINEIN_SWITCH, route->input == AUDIO_INPUT_LINE_IN);

    /* Volume, as a percentage of the control's hardware range. */
    volume_ctl = tiny_mixer_get_ctl_by_name(route->mixer, CTL_PLAYBACK_VOLUME);
    if (volume_ctl != NULL) {
        if (tiny_mixer_ctl_set_percent(volume_ctl, 0, route->volume) != 0)
            LOG_WARN("failed to set playback volume");
    } else {
        LOG_WARN("mixer control '%s' not found, skipping volume", CTL_PLAYBACK_VOLUME);
    }

    /*
     * Mute. On most ASoC codecs this "switch" control means
     * "playback enabled" (1 = audible, 0 = muted) - i.e. it is the
     * inverse of our mute flag. Flip the polarity here if your
     * codec's control works the other way.
     */
    apply_switch(route->mixer, CTL_PLAYBACK_MUTE, !route->mute);

    return AUDIO_OK;
}
