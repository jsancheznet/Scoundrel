#pragma once

#include <string>

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>

#include "typedefs.h"
#include "log.h"

// TODO: Despues de testear, crear un comentario explicando como funciona

typedef i32 sound;

const i16 MaxSoundCount = 64;

enum audio_channel
{
    Channel_Music = 0,
    Channel_SFX = 1,
    Channel_Invalid
};

struct sound_slot
{
    // TODO: Order for optimal alignment
    b32              InUse = false; // Wether the slot is currently in use
    i16              Generation = 0;
    std::string      Filename; // Sound Filename
    b32              Repeats; // Should the sound repeat?
    audio_channel    Channel;
    SDL_AudioSpec    Spec;
    SDL_AudioStream *Stream;
    u8              *AudioDataBuffer;
    u32              AudioDataBufferLength;
};

struct audio_system
{
    void Init();

    void PlayChannel(audio_channel Channel);
    void PauseChannel(audio_channel Channel);
    void SetChannelVolume(audio_channel Channel, f32 Volume);

    void Play(sound Handle);
    void Pause(sound Handle);
    void SetSoundVolume(sound Handle, float Volume);
    void SetRepeat(sound Handle, b32 ShouldRepeat);

    void PauseAll();
    void ResumeAll();

    void SetGlobalVolume(f32 Volume);

    sound CreateSound(const std::string &Path, audio_channel Channel);

    void DestroySound(sound Handle);

private:

    SDL_AudioDeviceID Channels[2];
    sound_slot Sounds[MaxSoundCount];

    //
    // Handle Helpers
    //

    inline sound CreateHandle(i16 Index, i16 Generation);
    inline sound InvalidHandle();
    inline sound_slot *ResolveHandle(sound Handle);
    inline i16 GetIndexFromHandle(sound Handle);
    inline i16 GetGenerationFromHandle(sound Handle);
};

// This was made exclusively for repeating a sound, this function checks if all bytes have been played, and copies the
// sound again to the Stream
void SDLCALL AudioStreamGetCallback(void *UserData, SDL_AudioStream *Stream, int AdditionalAmount, int TotalAmount);
