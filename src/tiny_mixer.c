#include <stdlib.h>

#include "../include/tiny_mixer.h"
#include <tinyalsa/mixer.h>


struct tiny_mixer {
    struct mixer *handle;
};

//-------------------            Mixer
 
struct tiny_mixer *tiny_mixer_open(unsigned int card)
{
    struct tiny_mixer *tiny_mixer;

    tiny_mixer = malloc(sizeof(*tiny_mixer));
    if (tiny_mixer == NULL)
        return NULL;

    tiny_mixer->handle = mixer_open(card);

    if (tiny_mixer->handle == NULL) {
        free(tiny_mixer);
        return NULL;
    }

    return tiny_mixer;
}

void tiny_mixer_close(struct tiny_mixer *mixer)
{
    if (mixer == NULL)
        return;

    mixer_close(mixer->handle);

    free(mixer);
}

const char *tiny_mixer_get_name(const struct tiny_mixer *mixer)
{
    if (mixer == NULL)
        return NULL;

    return mixer_get_name(mixer->handle);
}

unsigned int tiny_mixer_get_num_ctls(const struct tiny_mixer *mixer)
{
    if (mixer == NULL)
        return 0;

    return mixer_get_num_ctls(mixer->handle);
}

unsigned int tiny_mixer_get_num_ctls_by_name(const struct tiny_mixer *mixer, const char *name)
{
    if (mixer == NULL || name == NULL)
        return 0;

    return mixer_get_num_ctls_by_name(mixer->handle, name);
}


//------------------------        Mixer controls

struct tiny_mixer_ctl *tiny_mixer_get_ctl(struct tiny_mixer *mixer, unsigned int id)
{
    if (mixer == NULL)
        return NULL;

    return (struct tiny_mixer_ctl *)mixer_get_ctl(mixer->handle,id);
}

struct tiny_mixer_ctl *tiny_mixer_get_ctl_by_name(
    struct tiny_mixer *mixer,
    const char *name)
{
    if (mixer == NULL || name == NULL)
        return NULL;

    return (struct tiny_mixer_ctl *)mixer_get_ctl_by_name(mixer->handle,name);
}


struct tiny_mixer_ctl *tiny_mixer_get_ctl_by_name_and_index(struct tiny_mixer *mixer, const char *name, unsigned int index)
{
    if (mixer == NULL || name == NULL)
        return NULL;

    return (struct tiny_mixer_ctl *)mixer_get_ctl_by_name_and_index(mixer->handle, name, index);
}


//-------------------------       Mixer control management

int tiny_mixer_add_new_ctls(struct tiny_mixer *mixer)
{
    if (mixer == NULL)
        return -1;

    return mixer_add_new_ctls(mixer->handle);
}


//-------------------------        Mixer events 

int tiny_mixer_subscribe_events(struct tiny_mixer *mixer, int subscribe)
{
    if (mixer == NULL)
        return -1;

    return mixer_subscribe_events(mixer->handle, subscribe);
}

int tiny_mixer_wait_event(struct tiny_mixer *mixer, int timeout)
{
    if (mixer == NULL)
        return -1;

    return mixer_wait_event(mixer->handle, timeout);
}


//-----------------         Mixer control information

unsigned int tiny_mixer_ctl_get_id(struct tiny_mixer_ctl *ctl)
{
    if (ctl == NULL)
        return 0;

    return mixer_ctl_get_id((struct mixer_ctl *)ctl);
}


const char *tiny_mixer_ctl_get_name(struct tiny_mixer_ctl *ctl)
{
    if (ctl == NULL)
        return NULL;

    return mixer_ctl_get_name((struct mixer_ctl *)ctl);
}


enum tiny_mixer_ctl_type tiny_mixer_ctl_get_type(struct tiny_mixer_ctl *ctl)
{
    enum mixer_ctl_type type;

    if (ctl == NULL)
        return TINY_MIXER_CTL_TYPE_UNKNOWN;

    type = mixer_ctl_get_type((struct mixer_ctl *)ctl);

    switch (type) {
    case MIXER_CTL_TYPE_BOOL:
        return TINY_MIXER_CTL_TYPE_BOOL;

    case MIXER_CTL_TYPE_INT:
        return TINY_MIXER_CTL_TYPE_INT;

    case MIXER_CTL_TYPE_ENUM:
        return TINY_MIXER_CTL_TYPE_ENUM;

    case MIXER_CTL_TYPE_BYTE:
        return TINY_MIXER_CTL_TYPE_BYTE;

    case MIXER_CTL_TYPE_IEC958:
        return TINY_MIXER_CTL_TYPE_IEC958;

    case MIXER_CTL_TYPE_INT64:
        return TINY_MIXER_CTL_TYPE_INT64;

    default:
        return TINY_MIXER_CTL_TYPE_UNKNOWN;
    }
}


const char *tiny_mixer_ctl_get_type_string(struct tiny_mixer_ctl *ctl)
{
    if (ctl == NULL)
        return NULL;

    return mixer_ctl_get_type_string((struct mixer_ctl *)ctl);
}


unsigned int tiny_mixer_ctl_get_num_values(struct tiny_mixer_ctl *ctl)
{
    if (ctl == NULL)
        return 0;

    return mixer_ctl_get_num_values((struct mixer_ctl *)ctl);
}


unsigned int tiny_mixer_ctl_get_num_enums(struct tiny_mixer_ctl *ctl)
{
    if (ctl == NULL)
        return 0;

    return mixer_ctl_get_num_enums((struct mixer_ctl *)ctl);
}


const char *tiny_mixer_ctl_get_enum_string(struct tiny_mixer_ctl *ctl, unsigned int enum_id)
{
    if (ctl == NULL)
        return NULL;

    return mixer_ctl_get_enum_string((struct mixer_ctl *)ctl, enum_id);
}


void tiny_mixer_ctl_update(struct tiny_mixer_ctl *ctl)
{
    if (ctl == NULL)
        return;

    mixer_ctl_update((struct mixer_ctl *)ctl);
}


int tiny_mixer_ctl_is_access_tlv_rw(struct tiny_mixer_ctl *ctl)
{
    if (ctl == NULL)
        return 0;

    return mixer_ctl_is_access_tlv_rw((struct mixer_ctl *)ctl);
}



//-----------------          Mixer control values


int tiny_mixer_ctl_get_value(struct tiny_mixer_ctl *ctl, unsigned int id)
{
    if (ctl == NULL)
        return -1;

    return mixer_ctl_get_value((struct mixer_ctl *)ctl, id);
}


int tiny_mixer_ctl_set_value(struct tiny_mixer_ctl *ctl, unsigned int id, int value)
{
    if (ctl == NULL)
        return -1;

    return mixer_ctl_set_value((struct mixer_ctl *)ctl, id, value);
}


int tiny_mixer_ctl_get_percent(struct tiny_mixer_ctl *ctl, unsigned int id)
{
    if (ctl == NULL)
        return -1;

    return mixer_ctl_get_percent((struct mixer_ctl *)ctl, id);
}

int tiny_mixer_ctl_set_percent(struct tiny_mixer_ctl *ctl, unsigned int id, int percent)
{
    if (ctl == NULL)
        return -1;

    return mixer_ctl_set_percent((struct mixer_ctl *)ctl, id, percent);
}


//----------------         Mixer control arrays

int tiny_mixer_ctl_get_array(struct tiny_mixer_ctl *ctl, void *array, size_t count)
{
    if (ctl == NULL || array == NULL || count == 0)
        return -1;

    return mixer_ctl_get_array((struct mixer_ctl *)ctl, array, count);
}

int tiny_mixer_ctl_set_array(struct tiny_mixer_ctl *ctl, const void *array, size_t count)
{
    if (ctl == NULL || array == NULL || count == 0)
        return -1;

    return mixer_ctl_set_array((struct mixer_ctl *)ctl, array, count);
}


//-----------------------           Mixer enum controls

int tiny_mixer_ctl_set_enum_by_string(struct tiny_mixer_ctl *ctl, const char *string)
{
    if (ctl == NULL || string == NULL)
        return -1;

    return mixer_ctl_set_enum_by_string((struct mixer_ctl *)ctl, string);
}


//------------------------          Integer control range 

int tiny_mixer_ctl_get_range_min(struct tiny_mixer_ctl *ctl)
{
    if (ctl == NULL)
        return -1;

    return mixer_ctl_get_range_min((struct mixer_ctl *)ctl);
}

int tiny_mixer_ctl_get_range_max(struct tiny_mixer_ctl *ctl)
{
    if (ctl == NULL)
        return -1;

    return mixer_ctl_get_range_max((struct mixer_ctl *)ctl);
}