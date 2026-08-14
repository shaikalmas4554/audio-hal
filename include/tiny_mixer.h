#ifndef TINY_MIXER_H
#define TINY_MIXER_H

#include <stddef.h>

struct tiny_mixer;
struct tiny_mixer_ctl;


//---------------------      Mixer control types

enum tiny_mixer_ctl_type {
    TINY_MIXER_CTL_TYPE_BOOL,
    TINY_MIXER_CTL_TYPE_INT,
    TINY_MIXER_CTL_TYPE_ENUM,
    TINY_MIXER_CTL_TYPE_BYTE,
    TINY_MIXER_CTL_TYPE_IEC958,
    TINY_MIXER_CTL_TYPE_INT64,
    TINY_MIXER_CTL_TYPE_UNKNOWN,
};


//---------------------     Mixer

struct tiny_mixer *tiny_mixer_open(unsigned int card);

void tiny_mixer_close(struct tiny_mixer *mixer);

const char *tiny_mixer_get_name(const struct tiny_mixer *mixer);

unsigned int tiny_mixer_get_num_ctls(const struct tiny_mixer *mixer);

unsigned int tiny_mixer_get_num_ctls_by_name(const struct tiny_mixer *mixer, const char *name);

struct tiny_mixer_ctl *tiny_mixer_get_ctl(struct tiny_mixer *mixer, unsigned int id);

struct tiny_mixer_ctl *tiny_mixer_get_ctl_by_name(struct tiny_mixer *mixer, const char *name);

struct tiny_mixer_ctl *tiny_mixer_get_ctl_by_name_and_index(struct tiny_mixer *mixer, const char *name, unsigned int index);

int tiny_mixer_add_new_ctls(struct tiny_mixer *mixer);

int tiny_mixer_subscribe_events(struct tiny_mixer *mixer, int subscribe);

int tiny_mixer_wait_event(struct tiny_mixer *mixer, int timeout);


//----------------       Mixer control information

unsigned int tiny_mixer_ctl_get_id(struct tiny_mixer_ctl *ctl);

const char *tiny_mixer_ctl_get_name(struct tiny_mixer_ctl *ctl);

enum tiny_mixer_ctl_type tiny_mixer_ctl_get_type(struct tiny_mixer_ctl *ctl);

const char *tiny_mixer_ctl_get_type_string(struct tiny_mixer_ctl *ctl);

unsigned int tiny_mixer_ctl_get_num_values(struct tiny_mixer_ctl *ctl);

unsigned int tiny_mixer_ctl_get_num_enums(struct tiny_mixer_ctl *ctl);

const char *tiny_mixer_ctl_get_enum_string(struct tiny_mixer_ctl *ctl, unsigned int enum_id);

void tiny_mixer_ctl_update(struct tiny_mixer_ctl *ctl);

int tiny_mixer_ctl_is_access_tlv_rw(struct tiny_mixer_ctl *ctl);


//----------------      Mixer control values

int tiny_mixer_ctl_get_value(struct tiny_mixer_ctl *ctl, unsigned int id);

int tiny_mixer_ctl_set_value(struct tiny_mixer_ctl *ctl, unsigned int id, int value);

int tiny_mixer_ctl_get_percent(struct tiny_mixer_ctl *ctl, unsigned int id);

int tiny_mixer_ctl_set_percent(struct tiny_mixer_ctl *ctl, unsigned int id, int percent);



//----------------      Mixer control arrays

int tiny_mixer_ctl_get_array(struct tiny_mixer_ctl *ctl, void *array, size_t count);

int tiny_mixer_ctl_set_array(struct tiny_mixer_ctl *ctl, const void *array, size_t count);


//----------------      Mixer enum controls

int tiny_mixer_ctl_set_enum_by_string(struct tiny_mixer_ctl *ctl, const char *string);


//----------------      Integer control range

int tiny_mixer_ctl_get_range_min(struct tiny_mixer_ctl *ctl);

int tiny_mixer_ctl_get_range_max(struct tiny_mixer_ctl *ctl);


#endif