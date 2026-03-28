#ifndef MUSIC_TYPE_H
#define MUSIC_TYPE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

struct GK_Music {
    GK_MusicSetting set;
    GK_MusicObject *data;
    int size;
};

enum GK_MusicKind {
    GK_MUSIC_BACK,
    GK_MUSIC_SAMPLE
};

struct GK_MusicObject {
    GK_MusicKind kind;
    union {
        Mix_Music *back;
        char *sample;
    };
};

struct GK_MusicSetting {
    SDL_AudioDeviceID record;
    SDL_AudioSpec param_record;
    SDL_AudioDeviceID play;
    SDL_AudioSpec param_play;
};


#endif /* MUSIC_TYPE_H */
