#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>

#include "typedefs.h"
#include "log.h"

struct audio_channel
{
    SDL_AudioStream* Stream;
};

struct sound
{
    SDL_AudioSpec Spec;
    u8 *Buffer;
    u32 Length;
};

struct audio_system
{
    SDL_AudioDeviceID DeviceID;

    SDL_AudioSpec DeviceSpec;

    audio_channel Channel;

    void Init();

    void Play(sound Sound);

    void Pause();
    void Resume();

    void SetVolume(f32 Volume);

    sound CreateSound(const char* Path);
};
