#include "../include/error.h"


const char *audio_error_string(enum audio_error error)
{
    switch (error) {
    case AUDIO_OK:
        return "success";

    case AUDIO_ERROR_INVALID_PARAM:
        return "invalid parameter";

    case AUDIO_ERROR_NO_MEMORY:
        return "out of memory";

    case AUDIO_ERROR_NOT_READY:
        return "not ready";

    case AUDIO_ERROR_OPEN:
        return "open failed";

    case AUDIO_ERROR_IO:
        return "I/O error";

    case AUDIO_ERROR_NOT_SUPPORTED:
        return "not supported";

    case AUDIO_ERROR_NOT_FOUND:
        return "not found";

    case AUDIO_ERROR_INTERNAL:
        return "internal error";

    default:
        return "unknown error";
    }
}