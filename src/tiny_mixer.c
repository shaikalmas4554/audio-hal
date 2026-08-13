#ifndef TINY_MIXER_H
#define TINY_MIXER_H

struct tiny_mixer;
struct tiny_mixer_ctl;

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

int tiny_mixer_wait_event(struct tiny_mixer *mixer,int timeout);

#endif