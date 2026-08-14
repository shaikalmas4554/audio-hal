#include <stdio.h>

#include "../include/tiny_pcm.h"
#include "../include/tiny_mixer.h"


static void test_mixer(struct tiny_mixer *mixer)
{
    unsigned int num_ctls;
    unsigned int i;

    printf("\n");
    printf("========== MIXER TEST ==========\n");

    printf("Mixer name: %s\n",
           tiny_mixer_get_name(mixer));

    num_ctls = tiny_mixer_get_num_ctls(mixer);

    printf("Number of controls: %u\n", num_ctls);

    for (i = 0; i < num_ctls; i++) {
        struct tiny_mixer_ctl *ctl;
        enum tiny_mixer_ctl_type type;
        unsigned int num_values;

        ctl = tiny_mixer_get_ctl(mixer, i);

        if (ctl == NULL) {
            printf("\nControl %u: failed to get control\n", i);
            continue;
        }

        type = tiny_mixer_ctl_get_type(ctl);
        num_values = tiny_mixer_ctl_get_num_values(ctl);

        printf("\n");
        printf("Control %u\n", i);
        printf("  Name       : %s\n",
               tiny_mixer_ctl_get_name(ctl));

        printf("  Type       : %s\n",
               tiny_mixer_ctl_get_type_string(ctl));

        printf("  Num values : %u\n",
               num_values);

        /*
         * Integer controls
         */
        if (type == TINY_MIXER_CTL_TYPE_INT) {
            printf("  Range      : %d - %d\n",
                   tiny_mixer_ctl_get_range_min(ctl),
                   tiny_mixer_ctl_get_range_max(ctl));

            for (unsigned int value_id = 0;
                 value_id < num_values;
                 value_id++) {

                printf("  Value[%u]   : %d\n",
                       value_id,
                       tiny_mixer_ctl_get_value(
                           ctl,
                           value_id
                       ));
            }
        }

        /*
         * Boolean controls
         */
        else if (type == TINY_MIXER_CTL_TYPE_BOOL) {
            for (unsigned int value_id = 0;
                 value_id < num_values;
                 value_id++) {

                printf("  Value[%u]   : %d\n",
                       value_id,
                       tiny_mixer_ctl_get_value(
                           ctl,
                           value_id
                       ));
            }
        }

        /*
         * Enum controls
         */
        else if (type == TINY_MIXER_CTL_TYPE_ENUM) {
            unsigned int num_enums;
            unsigned int enum_id;

            num_enums = tiny_mixer_ctl_get_num_enums(ctl);

            printf("  Num enums   : %u\n", num_enums);

            for (enum_id = 0;
                 enum_id < num_enums;
                 enum_id++) {

                printf("  Enum[%u]    : %s\n",
                       enum_id,
                       tiny_mixer_ctl_get_enum_string(
                           ctl,
                           enum_id
                       ));
            }
        }
    }

    printf("\n");
    printf("========== MIXER TEST DONE ==========\n");
}


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
    struct tiny_mixer *mixer;


    /*
     * PCM test
     */

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


    /*
     * Mixer test
     */

    printf("\nOpening mixer...\n");

    mixer = tiny_mixer_open(0);

    if (mixer == NULL) {
        printf("tiny_mixer_open() failed\n");

        tiny_pcm_close(pcm);
        return 1;
    }

    printf("Mixer opened successfully\n");

    test_mixer(mixer);


    /*
     * Close
     */

    tiny_mixer_close(mixer);

    printf("Mixer closed\n");

    tiny_pcm_close(pcm);

    printf("PCM closed\n");

    return 0;
}