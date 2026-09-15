#pragma once

#include <string>

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>

#include "typedefs.h"
#include "log.h"

// TODO:
//    - Be able to set sounds on repeat

enum audio_channel
{
    AudioChannel_Music = 0,
    AudioChannel_SFX = 1,

    AudioChannel_Invalid
};

struct sound
{
    std::string Name;
    bool Repeats;
    audio_channel Channel;
    SDL_AudioSpec Spec;
    SDL_AudioStream* Stream;
    u8 *Buffer;
    u32 Length;
};

struct audio_system
{
    SDL_AudioDeviceID Channels[2];

    void Init();

    void PlayChannel(audio_channel Channel);
    void PauseChannel(audio_channel Channel);
    void SetChannelVolume(audio_channel Channel, f32 Volume);

    void Play(sound Sound);
    void Pause(sound Sound);
    void SetSoundVolume(sound Sound, float Volume);

    void PauseAll();
    void ResumeAll();

    void SetGlobalVolume(f32 Volume);

    sound CreateSound(const std::string &Path, audio_channel Channel, bool Repeats);
    void  DestroySound(sound &Sound);
};
