#ifndef ERROR_H
#define ERROR_H


enum audio_error {
    AUDIO_OK = 0,

    AUDIO_ERROR_INVALID_PARAM = -1,
    AUDIO_ERROR_NO_MEMORY = -2,
    AUDIO_ERROR_NOT_READY = -3,
    AUDIO_ERROR_OPEN = -4,
    AUDIO_ERROR_IO = -5,
    AUDIO_ERROR_NOT_SUPPORTED = -6,
    AUDIO_ERROR_NOT_FOUND = -7,
    AUDIO_ERROR_INTERNAL = -8,
};


const char *audio_error_string(enum audio_error error);


#endif