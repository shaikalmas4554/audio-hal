#include <stdio.h>

#include "../include/tiny_pcm.h"

int main(void)
{
    tiny_pcm_config_t config = {
        .channels = 2,
        .rate = 48000,
        .period_size = 1024,
        .period_count = 4,
        .format = TINY_PCM_FORMAT_S16_LE,
    };

    struct tiny_pcm *pcm;

    printf("Opening PCM...\n");

    pcm = tiny_pcm_open(
        0,
        0,
        TINY_PCM_PLAYBACK,
        &config
    );

    if (pcm == NULL) {
        printf("tiny_pcm_open() failed\n");
        return 1;
    }

    printf("PCM opened successfully\n");

    if (!tiny_pcm_is_ready(pcm)) {
        printf("PCM is not ready: %s\n",
               tiny_pcm_get_error(pcm));

        tiny_pcm_close(pcm);
        return 1;
    }

    printf("PCM is ready\n");

    tiny_pcm_close(pcm);

    printf("PCM closed\n");

    return 0;
}