#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "../include/tiny_pcm.h"
#include "../include/tiny_mixer.h"


/* -------------------------------------------------------------------------
 * Defaults
 * ------------------------------------------------------------------------- */

#define DEFAULT_CARD         0
#define DEFAULT_DEVICE       0
#define DEFAULT_RATE         48000
#define DEFAULT_CHANNELS     2
#define DEFAULT_PERIOD_SIZE  1024
#define DEFAULT_PERIOD_COUNT 4
#define DEFAULT_MIXER_CARD   -1


/* -------------------------------------------------------------------------
 * Application configuration
 * ------------------------------------------------------------------------- */

struct test_config {
    unsigned int card;
    unsigned int device;

    unsigned int rate;
    unsigned int channels;
    unsigned int period_size;
    unsigned int period_count;

    int mixer_card;

    int playback;

    tiny_pcm_format_t format;
};


/* -------------------------------------------------------------------------
 * Help
 * ------------------------------------------------------------------------- */

static void print_usage(const char *program)
{
    printf("\n");
    printf("Usage:\n");
    printf("  %s [options]\n", program);

    printf("\nPCM options:\n");
    printf("  -c, --card <n>          PCM card number\n");
    printf("  -d, --device <n>        PCM device number\n");
    printf("  -r, --rate <Hz>         Sample rate\n");
    printf("  -C, --channels <n>      Number of channels\n");
    printf("  -p, --period-size <n>   Period size\n");
    printf("  -n, --period-count <n>  Period count\n");
    printf("  -f, --format <format>   PCM format\n");

    printf("\nDirection:\n");
    printf("      --playback          Open PCM for playback\n");
    printf("      --capture           Open PCM for capture\n");

    printf("\nMixer options:\n");
    printf("  -m, --mixer-card <n>    Mixer card number\n");

    printf("\nOther:\n");
    printf("  -h, --help              Show this help\n");

    printf("\nSupported formats:\n");
    printf("  S16_LE\n");

    printf("\nDefaults:\n");
    printf("  card         : %d\n", DEFAULT_CARD);
    printf("  device       : %d\n", DEFAULT_DEVICE);
    printf("  rate         : %d Hz\n", DEFAULT_RATE);
    printf("  channels     : %d\n", DEFAULT_CHANNELS);
    printf("  period size  : %d\n", DEFAULT_PERIOD_SIZE);
    printf("  period count : %d\n", DEFAULT_PERIOD_COUNT);
    printf("  mixer card   : same as PCM card\n");
    printf("  direction    : playback\n");
    printf("  format       : S16_LE\n");

    printf("\nExamples:\n");

    printf("\n  Bluetooth SCO playback:\n");
    printf("    %s --card 0 --device 0 --playback \\\n"
           "--rate 16000 --channels 1\n",
           program);

    printf("\n  MICFIL capture:\n");
    printf("    %s --card 1 --device 0 --capture \\\n"
           "--rate 48000 --channels 1\n",
           program);

    printf("\n  WM8524 playback:\n");
    printf("    %s --card 3 --device 0 --playback \\\n"
           "--rate 48000 --channels 2\n",
           program);

    printf("\n");
}


/* -------------------------------------------------------------------------
 * Parse unsigned integer
 * ------------------------------------------------------------------------- */

static int parse_uint(const char *str, unsigned int *value)
{
    unsigned int val;

    if (str == NULL || value == NULL)
        return -1;

    if (sscanf(str, "%u", &val) != 1)
        return -1;

    *value = val;

    return 0;
}



/* -------------------------------------------------------------------------
 * Parse PCM format
 * ------------------------------------------------------------------------- */

static int parse_format(const char *str, tiny_pcm_format_t *format)
{
    if (str == NULL || format == NULL)
        return -1;

    if (strcmp(str, "S16_LE") == 0) {
        *format = TINY_PCM_FORMAT_S16_LE;
        return 0;
    }

    return -1;
}


/* -------------------------------------------------------------------------
 * Parse command line
 * ------------------------------------------------------------------------- */

static int parse_arguments(
    int argc,
    char **argv,
    struct test_config *config)
{
    int option_index = 0;
    int c;

    static const struct option long_options[] = {
        {"card",         required_argument, 0, 'c'},
        {"device",       required_argument, 0, 'd'},
        {"rate",         required_argument, 0, 'r'},
        {"channels",     required_argument, 0, 'C'},
        {"period-size",  required_argument, 0, 'p'},
        {"period-count", required_argument, 0, 'n'},
        {"format",       required_argument, 0, 'f'},
        {"playback",     no_argument,       0,  1 },
        {"capture",      no_argument,       0,  2 },
        {"mixer-card",   required_argument, 0, 'm'},
        {"help",         no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    while ((c = getopt_long(
                argc,
                argv,
                "c:d:r:C:p:n:f:m:h",
                long_options,
                &option_index)) != -1) {

        switch (c) {

        case 'c':
            if (parse_uint(optarg, &config->card) < 0) {
                fprintf(stderr,
                        "Invalid card: %s\n",
                        optarg);
                return -1;
            }
            break;

        case 'd':
            if (parse_uint(optarg, &config->device) < 0) {
                fprintf(stderr,
                        "Invalid device: %s\n",
                        optarg);
                return -1;
            }
            break;

        case 'r':
            if (parse_uint(optarg, &config->rate) < 0) {
                fprintf(stderr,
                        "Invalid rate: %s\n",
                        optarg);
                return -1;
            }
            break;

        case 'C':
            if (parse_uint(optarg, &config->channels) < 0) {
                fprintf(stderr,
                        "Invalid channels: %s\n",
                        optarg);
                return -1;
            }
            break;

        case 'p':
            if (parse_uint(optarg, &config->period_size) < 0) {
                fprintf(stderr,
                        "Invalid period size: %s\n",
                        optarg);
                return -1;
            }
            break;

        case 'n':
            if (parse_uint(optarg, &config->period_count) < 0) {
                fprintf(stderr,
                        "Invalid period count: %s\n",
                        optarg);
                return -1;
            }
            break;

        case 'f':
            if (parse_format(optarg, &config->format) < 0) {
                fprintf(stderr,
                        "Unsupported format: %s\n",
                        optarg);
                return -1;
            }
            break;

        case 'm':
            if (parse_uint(optarg,
                           (unsigned int *)&config->mixer_card) < 0) {
                fprintf(stderr,
                        "Invalid mixer card: %s\n",
                        optarg);
                return -1;
            }
            break;

        case 1:
            config->playback = 1;
            break;

        case 2:
            config->playback = 0;
            break;

        case 'h':
            print_usage(argv[0]);
            exit(0);

        case '?':
        default:
            print_usage(argv[0]);
            return -1;
        }
    }

    return 0;
}


/* -------------------------------------------------------------------------
 * Mixer test
 * ------------------------------------------------------------------------- */

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

            printf("  Num enums  : %u\n", num_enums);

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


/* -------------------------------------------------------------------------
 * Print selected configuration
 * ------------------------------------------------------------------------- */

static void print_config(const struct test_config *config)
{
    printf("\n");
    printf("========== PCM CONFIG ==========\n");

    printf("Card         : %u\n", config->card);
    printf("Device       : %u\n", config->device);

    printf("Direction    : %s\n",
           config->playback ? "playback" : "capture");

    printf("Rate         : %u Hz\n",
           config->rate);

    printf("Channels     : %u\n",
           config->channels);

    printf("Period size  : %u\n",
           config->period_size);

    printf("Period count : %u\n",
           config->period_count);

    printf("Format       : S16_LE\n");

    if (config->mixer_card >= 0)
        printf("Mixer card   : %d\n",
               config->mixer_card);
    else
        printf("Mixer card   : %u (same as PCM card)\n",
               config->card);

    printf("================================\n");
}


/* -------------------------------------------------------------------------
 * Main
 * ------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
    struct test_config config;

    tiny_pcm_config_t pcm_config;

    struct tiny_pcm *pcm = NULL;
    struct tiny_mixer *mixer = NULL;

    int mixer_card;


    /*
     * Default configuration
     */

    memset(&config, 0, sizeof(config));

    config.card = DEFAULT_CARD;
    config.device = DEFAULT_DEVICE;

    config.rate = DEFAULT_RATE;
    config.channels = DEFAULT_CHANNELS;

    config.period_size = DEFAULT_PERIOD_SIZE;
    config.period_count = DEFAULT_PERIOD_COUNT;

    config.mixer_card = DEFAULT_MIXER_CARD;

    config.playback = 1;

    config.format = TINY_PCM_FORMAT_S16_LE;


    /*
     * Parse command line
     */

    if (parse_arguments(argc, argv, &config) < 0)
        return 1;


    /*
     * Mixer defaults to PCM card
     */

    if (config.mixer_card < 0)
        mixer_card = (int)config.card;
    else
        mixer_card = config.mixer_card;


    /*
     * Print configuration
     */

    print_config(&config);


    /*
     * Build Tiny PCM configuration
     */

    pcm_config.channels = config.channels;
    pcm_config.rate = config.rate;
    pcm_config.period_size = config.period_size;
    pcm_config.period_count = config.period_count;
    pcm_config.format = config.format;


    /*
     * PCM test
     */

    printf("\nOpening PCM...\n");

    pcm = tiny_pcm_open(
        config.card,
        config.device,
        config.playback
            ? TINY_PCM_PLAYBACK
            : TINY_PCM_CAPTURE,
        &pcm_config
    );

    if (pcm == NULL) {
        printf("tiny_pcm_open() failed\n");
        return 1;
    }

    printf("PCM opened successfully\n");


    /*
     * Check PCM status
     */

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

    printf("\nOpening mixer card %d...\n",
           mixer_card);

    mixer = tiny_mixer_open(mixer_card);

    if (mixer == NULL) {

        printf("tiny_mixer_open() failed\n");

        tiny_pcm_close(pcm);

        return 1;
    }

    printf("Mixer opened successfully\n");


    /*
     * Enumerate mixer controls
     */

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
