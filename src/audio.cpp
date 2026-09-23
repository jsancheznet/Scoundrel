#include <glm/glm.hpp>

#include <cstring>

#include "helpers.h"
#include "audio.h"
#include "asset_manager.h"

extern asset_manager AssetMgr;

void audio_system::Init()
{
    Log(Info, "AudioSystem::Init()");

    { // Print Audio Devices to Log
        i32 AudioDeviceCount = 0;
        SDL_AudioDeviceID *Devices = SDL_GetAudioPlaybackDevices(&AudioDeviceCount);
        Log(Info, "AudioSystem::Init - Printing available audio devices");
        for(int i = 0; i < AudioDeviceCount; ++i)
        {
            Log(Info, "\tDevice %d: %s", i, SDL_GetAudioDeviceName(Devices[i]));
        }
    }


    // Create all the channels on the default device
    for(int i = 0; i < Channel_Count; ++i)
    {
        Channels[i] = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    }

    ResumeAllChannels();
}

void audio_system::PlayChannel(audio_channel Channel)
{
    Log(Info, "audio_system::PlayChannel - Resuming playback on channel %d", Channel);
    SDL_ResumeAudioDevice(Channels[Channel]);
}

void audio_system::PauseChannel(audio_channel Channel)
{
    Log(Info, "audio_system::PlayChannel - Pausing playback on channel %d", Channel);
    SDL_PauseAudioDevice(Channels[Channel]);
}

void audio_system::SetChannelVolume(audio_channel Channel, f32 Volume)
{
    Volume = glm::clamp(Volume, 0.0f, 1.0f);

    Log(Info, "audio_system::SetChannelVolume - Channel: %d, Volume: %.2f", Channel, Volume);

    SDL_SetAudioDeviceGain(Channels[Channel], Volume);
}

void audio_system::Play(u64 Handle)
{
    sound *Sound = (sound*)AssetMgr.ResolveHandle(Handle);
    if(Sound == nullptr)
        return;

    // Find a free stream
    i32 Index = -1;
    for(i32 i = 0; i < MaxConcurrentStreams; ++i)
    {
        i32 BytesLeft = SDL_GetAudioStreamAvailable(Sound->Streams[i]);
        if(BytesLeft < 1)
        {
            Index = i;

            // If the stream is not null, it means it has a sound that has ended, so clear it so we do not leak.
            if(Sound->Streams[i] != nullptr)
            {
                SDL_ClearAudioStream(Sound->Streams[i]);
                SDL_UnbindAudioStream(Sound->Streams[i]);
            }

            break;
        }
    }

    if(Index < 0)
    {
        Log(Info, "audio_system::Play() - Could not find a free Audio Stream to use, try raising MaxConcurrentStreams");
        return;
    }

    Log(Info, "audio_system::Play() - Creating sound stream using index: %d", Index);

    SDL_AudioSpec DeviceSpec;
    SDL_GetAudioDeviceFormat(Channels[Sound->Channel], &DeviceSpec, NULL);
    Sound->Streams[Index] = SDL_CreateAudioStream(&Sound->AudioSpec, &DeviceSpec);
    SDL_PutAudioStreamData(Sound->Streams[Index], Sound->DataBuffer, Sound->DataBufferLength);
    SDL_BindAudioStream(Channels[Sound->Channel], Sound->Streams[Index]);

    if(Sound->Repeats)
    {
        // Set the callback
        SDL_SetAudioStreamGetCallback(Sound->Streams[Index], audio_system::AudioStreamGetCallback, Sound);
    }
}

void audio_system::Pause(u64 Handle)
{
    sound *Sound = (sound*)AssetMgr.ResolveHandle(Handle);
    if(Sound == nullptr)
        return;

    Log(Info, "audio_system::Pause() - Pausing all instances of sound");
    SDL_UnbindAudioStreams(Sound->Streams, MaxConcurrentStreams);
}


void audio_system::Resume(u64 Handle)
{
    sound *Sound = (sound*)AssetMgr.ResolveHandle(Handle);
    if(Sound == nullptr)
        return;

    Log(Info, "audio_system::Resume() - Resuming all instances of sound");

    for(i32 i = 0; i < MaxConcurrentStreams; ++i)
    {
        if(Sound->Streams[i] != nullptr)
        {
            SDL_BindAudioStream(Channels[Sound->Channel], Sound->Streams[i]);
        }
    }
}

void audio_system::SetRepeat(u64 Handle, b32 ShouldRepeat)
{
    sound *Sound = (sound*)AssetMgr.ResolveHandle(Handle);
    if(Sound == nullptr)
        return;

    if(ShouldRepeat != Sound->Repeats)
    {
        Log(Info, "audio_system::SetRepeat()");
        Sound->Repeats = ShouldRepeat;

        for(i32 i = 0; i < MaxConcurrentStreams; ++i)
        {
            if(Sound->Streams[i] != nullptr)
            {
                if(ShouldRepeat)
                {
                    // Assign the callback that copies the audio again once it finishes
                    SDL_SetAudioStreamGetCallback(Sound->Streams[i], AudioStreamGetCallback, Sound);
                }
                else
                {
                    // Turn off the callback
                    SDL_SetAudioStreamGetCallback(Sound->Streams[i], NULL, Sound);
                }
            }
        }
    }
}

void audio_system::PauseAllChannels()
{
    Log(Info, "AudioSystem::PauseAll - Pausing All Sound Devices");

    for(int i = 0; i < Channel_Count; ++i)
    {
        SDL_PauseAudioDevice(Channels[i]);
    }
}

void audio_system::ResumeAllChannels()
{
    Log(Info, "AudioSystem::ResumeAll - Resuming All Sound Devices");

    for(int i = 0; i < Channel_Count; ++i)
    {
        SDL_ResumeAudioDevice(Channels[i]);
    }
}

void SDLCALL audio_system::AudioStreamGetCallback(void *UserData, SDL_AudioStream *Stream, int AdditionalAmount, int TotalAmount)
{
    // This function only gets called for repeating sounds. It checks if the sound has ended and copies the audio data
    // to be played again

    // NOTE: When this gets called after a sound has ended, it restarts it!, Should it?

    sound *Sound = (sound*)UserData;

    for(i32 i = 0; i < MaxConcurrentStreams; ++i)
    {
        if(Sound->Streams[i] != nullptr)
        {
            int SoundBytesRemaining = SDL_GetAudioStreamAvailable(Sound->Streams[i]);
            if(SoundBytesRemaining == 0)
            {
                SDL_PutAudioStreamData(Sound->Streams[i], Sound->DataBuffer, Sound->DataBufferLength);
            }
        }
    }
}
