#pragma once

#include <string>

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>

#include "typedefs.h"
#include "log.h"

constexpr i16 MaxConcurrentStreams = 8;

enum audio_channel
{
    Channel_None = 0,
    Channel_Music = 1,
    Channel_SFX = 2,

    Channel_Count
};

struct sound
{
    audio_channel Channel;
    bool Repeats;
    SDL_AudioSpec AudioSpec;
    u32 DataBufferLength;
    SDL_AudioStream *Streams[MaxConcurrentStreams];
    u8  *DataBuffer;
};

struct audio_system
{
    void Init();

    // Channel Functions
    void PlayChannel(audio_channel Channel);
    void PauseChannel(audio_channel Channel);
    void SetChannelVolume(audio_channel Channel, f32 Volume);
    void PauseAllChannels();
    void ResumeAllChannels();

    // Sound Functions
    void Play(u64 Handle);
    void Pause(u64 Handle);
    void Resume(u64 Handle);
    void SetRepeat(u64 Handle, b32 ShouldRepeat);// FIX

private:

    SDL_AudioDeviceID Channels[Channel_Count];

    // This was made exclusively for repeating a sound, this function checks if all bytes have been played, and copies the
    // sound again to the Stream
    static void SDLCALL AudioStreamGetCallback(void *UserData, SDL_AudioStream *Stream, int AdditionalAmount, int TotalAmount);
};
