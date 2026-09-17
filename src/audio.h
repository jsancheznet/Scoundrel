#pragma once

#include <string>

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>

// Audio here has three layers: devices, assets, and streams.
//
// At the bottom are two SDL audio devices, one for music and one for sound effects. They work like mixing buses
// -- pause one or turn its volume down and everything playing through it follows. SDL owns the actual playback
// and pulls audio on its own thread; nothing in this file mixes samples by hand.
//
// In the middle are assets. A sound file is decoded into memory once and stored in a fixed pool, and the caller
// gets back a handle instead of a pointer. The handle carries the slot it lives in plus a generation counter,
// which is bumped when the sound is destroyed, so an old handle is recognized as stale rather than quietly
// pointing at whatever sound took over that slot.
//
// On top are streams. Playing a sound doesn't copy it; it hands the asset's samples to one of a small number of
// SDL streams the asset keeps, each bound to its channel. That's what lets the same sound play several times at
// once -- one stream per instance, all sharing the same sample data -- with the array size capping how many can
// overlap. Looping is not special-cased either: a repeating sound just installs a callback that hands SDL the
// samples again when the stream runs dry.

#include "typedefs.h"
#include "log.h"

constexpr i16 MaxSoundAssetCount = 32;
constexpr i16 MaxConcurrentStreams = 8;

enum audio_channel
{
    Channel_Invalid = -1,
    Channel_Music = 0,
    Channel_SFX = 1,

    Channel_Count
};

typedef i32 sound;

struct sound_asset
{
    std::string Filename;
    audio_channel Channel;
    b32 Repeats;
    f32 Volume;
    SDL_AudioSpec AudioSpec;
    u32 DataBufferLength;
    SDL_AudioStream *Streams[MaxConcurrentStreams] = {};
    i16 Generation = 0;
    u8  *DataBuffer = nullptr;
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
    void SetGlobalVolume(f32 Volume);

    // Sound Functions
    sound CreateSound(const std::string &Filepath, audio_channel Channel);
    void DestroySound(sound Handle);
    void Play(sound Handle);
    void Pause(sound Handle);
    void Resume(sound Handle);
    void SetSoundVolume(sound Handle, float Volume);
    void SetRepeat(sound Handle, b32 ShouldRepeat);

private:

    SDL_AudioDeviceID Channels[Channel_Count];

    u32 AssetsCount = 0;
    sound_asset Assets[MaxSoundAssetCount];

    //
    // Handle Helpers
    //

    inline sound CreateHandle(i16 Index, i16 Generation);
    inline sound InvalidHandle();
    inline sound_asset *ResolveHandle(sound Handle);
    inline i16 GetIndexFromHandle(sound Handle);
    inline i16 GetGenerationFromHandle(sound Handle);
};

// This was made exclusively for repeating a sound, this function checks if all bytes have been played, and copies the
// sound again to the Stream
void SDLCALL AudioStreamGetCallback(void *UserData, SDL_AudioStream *Stream, int AdditionalAmount, int TotalAmount);
