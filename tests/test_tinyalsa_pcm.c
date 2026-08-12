#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <tinyalsa/asoundlib.h>

#define CARD            3
#define DEVICE          0

#define SAMPLE_RATE     48000
#define CHANNELS        2
#define DURATION_SEC    5
#define FREQUENCY_HZ    1000

#define PERIOD_SIZE     1024
#define PERIOD_COUNT    4

static void generate_tone(
        int16_t *buffer,
        size_t frames)
{
    static double phase = 0.0;

    const double phase_increment =
        2.0 * M_PI * FREQUENCY_HZ / SAMPLE_RATE;

    for (size_t i = 0; i < frames; i++) {
        int16_t sample =
            (int16_t)(0.25 * 32767.0 * sin(phase));

        buffer[2 * i]     = sample;
        buffer[2 * i + 1] = sample;

        phase += phase_increment;

        if (phase >= 2.0 * M_PI)
            phase -= 2.0 * M_PI;
    }
}

int main(void)
{
    struct pcm_config config = {
        .channels = CHANNELS,
        .rate = SAMPLE_RATE,
        .period_size = PERIOD_SIZE,
        .period_count = PERIOD_COUNT,
        .format = PCM_FORMAT_S16_LE,
        .start_threshold = 0,
        .stop_threshold = 0,
        .silence_threshold = 0,
    };

    printf("TinyALSA direct PCM test\n");
    printf("------------------------\n");
    printf("Card          : %d\n", CARD);
    printf("Device        : %d\n", DEVICE);
    printf("Rate          : %d Hz\n", SAMPLE_RATE);
    printf("Channels      : %d\n", CHANNELS);
    printf("Format        : S16_LE\n");
    printf("Period size   : %d\n", PERIOD_SIZE);
    printf("Period count  : %d\n", PERIOD_COUNT);
    printf("Duration      : %d seconds\n", DURATION_SEC);

    printf("\nOpening PCM...\n");

    struct pcm *pcm = pcm_open(
        CARD,
        DEVICE,
        PCM_OUT,
        &config
    );

    if (pcm == NULL) {
        fprintf(stderr, "ERROR: pcm_open returned NULL\n");
        return EXIT_FAILURE;
    }

    if (!pcm_is_ready(pcm)) {
        fprintf(stderr,
                "ERROR: PCM is not ready: %s\n",
                pcm_get_error(pcm));

        pcm_close(pcm);
        return EXIT_FAILURE;
    }

    printf("PCM opened successfully.\n");

    size_t frames_per_buffer = PERIOD_SIZE;

    size_t bytes_per_frame =
        CHANNELS * sizeof(int16_t);

    size_t buffer_bytes =
        frames_per_buffer * bytes_per_frame;

    int16_t *buffer = malloc(buffer_bytes);

    if (buffer == NULL) {
        fprintf(stderr, "ERROR: buffer allocation failed\n");
        pcm_close(pcm);
        return EXIT_FAILURE;
    }

    printf("Starting playback...\n");

    size_t total_frames =
        SAMPLE_RATE * DURATION_SEC;

    size_t frames_written = 0;

    while (frames_written < total_frames) {

        size_t frames_remaining =
            total_frames - frames_written;

        size_t frames =
            frames_remaining < frames_per_buffer
                ? frames_remaining
                : frames_per_buffer;

        generate_tone(buffer, frames);

        unsigned int bytes =
            frames * bytes_per_frame;

        int ret = pcm_write(
            pcm,
            buffer,
            bytes
        );

        if (ret < 0) {
            fprintf(stderr,
                    "\nERROR: pcm_write failed: %s\n",
                    pcm_get_error(pcm));

            free(buffer);
            pcm_close(pcm);
            return EXIT_FAILURE;
        }

        frames_written += frames;
    }

    printf("Playback completed.\n");

    free(buffer);

    pcm_close(pcm);

    printf("PCM closed.\n");
    printf("Test PASSED.\n");

    return EXIT_SUCCESS;
}
