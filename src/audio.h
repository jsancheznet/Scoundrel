#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>

#include "typedefs.h"
#include "log.h"

struct audio_channel
{
    SDL_AudioStream* Stream;
};

struct audio_system
{
    SDL_AudioDeviceID DeviceID;

    audio_channel Channels;

    void Init();

    void Pause();
    void Resume();

    void SetVolume(f32 Volume);
};

struct sound
{
    SDL_AudioSpec Spec;
    u8 *Buffer;
    u32 Length;
};

sound CreateSound(const char* Path);
