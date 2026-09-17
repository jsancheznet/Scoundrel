#include <glm/glm.hpp>

#include "audio.h"

void audio_system::Init()
{
    Log(Info, "AudioSystem::Init()");

    i32 AudioDeviceCount = 0;
    SDL_AudioDeviceID *Devices = SDL_GetAudioPlaybackDevices(&AudioDeviceCount);
    Log(Info, "AudioSystem::Init - Printing available audio devices");
    for(int i = 0; i < AudioDeviceCount; ++i)
    {
        Log(Info, "\tDevice %d: %s", i, SDL_GetAudioDeviceName(Devices[i]));
    }

    // Create all the channels on the default device
    Channels[Channel_Music] = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    Channels[Channel_SFX]   = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);

    // Do not start playing sound at startup, unpause it manually
    PauseAllChannels();
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

void audio_system::Play(sound Handle)
{
    sound_asset *Asset = ResolveHandle(Handle);
    if(Asset == nullptr)
        return;

    Log(Info, "audio::system::Play() - Playing sound %s", Asset->Filename.c_str());

    // Find a free stream
    i32 Index = -1;
    for(i32 i = 0; i < MaxConcurrentStreams; ++i)
    {
        i32 BytesLeft = SDL_GetAudioStreamAvailable(Asset->Streams[i]);
        if(BytesLeft < 1)
        {
            Index = i;

            // If the stream is not null, it means it has a sound that has ended, so clear it so we do not leak.
            if(Asset->Streams[i] != nullptr)
            {
                SDL_ClearAudioStream(Asset->Streams[i]);
                SDL_UnbindAudioStream(Asset->Streams[i]);
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
    SDL_GetAudioDeviceFormat(Channels[Asset->Channel], &DeviceSpec, NULL);
    Asset->Streams[Index] = SDL_CreateAudioStream(&Asset->AudioSpec, &DeviceSpec);
    SDL_PutAudioStreamData(Asset->Streams[Index], Asset->DataBuffer, Asset->DataBufferLength);
    SDL_BindAudioStream(Channels[Asset->Channel], Asset->Streams[Index]);
    SDL_SetAudioStreamGain(Asset->Streams[Index], Asset->Volume);

    if(Asset->Repeats)
    {
        // Set the callback
        SDL_SetAudioStreamGetCallback(Asset->Streams[Index], AudioStreamGetCallback, Asset);
    }
}

void audio_system::Pause(sound Handle)
{
    sound_asset *Asset = ResolveHandle(Handle);
    if(Asset == nullptr)
        return;

    Log(Info, "audio_system::Pause() - Pausing all instances of sound %s", Asset->Filename.c_str());
    SDL_UnbindAudioStreams(Asset->Streams, MaxConcurrentStreams);
}


void audio_system::Resume(sound Handle)
{
    sound_asset *Asset = ResolveHandle(Handle);
    if(Asset == nullptr)
        return;

    Log(Info, "audio_system::Resume() - Resuming all instances of sound %s", Asset->Filename.c_str());

    for(i32 i = 0; i < MaxConcurrentStreams; ++i)
    {
        if(Asset->Streams[i] != nullptr)
        {
            SDL_BindAudioStream(Channels[Asset->Channel], Asset->Streams[i]);
        }
    }
}

void audio_system::SetSoundVolume(sound Handle, f32 Volume)
{
    sound_asset *Asset = ResolveHandle(Handle);
    if(Asset == nullptr)
        return;

    Asset->Volume = glm::clamp(Volume, 0.0f, 1.0f);

    Log(Info, "audio_system::SetSoundVolume - Sound: %s, Volume: %f", Asset->Filename.c_str(), Asset->Volume);

    for(i32 i = 0; i < MaxConcurrentStreams; ++i)
    {
        if(Asset->Streams[i] != nullptr)
        {
            SDL_SetAudioStreamGain(Asset->Streams[i], Asset->Volume);
        }
    }
}

void audio_system::SetRepeat(sound Handle, b32 ShouldRepeat)
{
    sound_asset *Asset = ResolveHandle(Handle);
    if(Asset == nullptr)
        return;

    if(ShouldRepeat != Asset->Repeats)
    {
        Log(Info, "audio_system::SetRepeat() - Asset: %s, Repeat: %s", Asset->Filename.c_str(), ShouldRepeat ? "true" : "false");
        Asset->Repeats = ShouldRepeat;

        for(i32 i = 0; i < MaxConcurrentStreams; ++i)
        {
            if(Asset->Streams[i] != nullptr)
            {
                if(ShouldRepeat)
                {
                    // Assign the callback that copies the audio again once it finishes
                    SDL_SetAudioStreamGetCallback(Asset->Streams[i], AudioStreamGetCallback, Asset);
                }
                else
                {
                    // Turn off the callback
                    SDL_SetAudioStreamGetCallback(Asset->Streams[i], NULL, Asset);
                }
            }
        }
    }
}

void audio_system::PauseAllChannels()
{
    Log(Info, "AudioSystem::PauseAll - Pausing All Sound Devices");
    SDL_PauseAudioDevice(Channels[Channel_Music]);
    SDL_PauseAudioDevice(Channels[Channel_SFX]);
}

void audio_system::ResumeAllChannels()
{
    Log(Info, "AudioSystem::ResumeAll - Resuming All Sound Devices");

    SDL_ResumeAudioDevice(Channels[Channel_Music]);
    SDL_ResumeAudioDevice(Channels[Channel_SFX]);
}

void audio_system::SetGlobalVolume(f32 Volume)
{
    Log(Info, "AudioSystem::SetGlobalVolume() - Volume: %.2f", Volume);

    Volume = glm::clamp(Volume, 0.0f, 1.0f);

    SDL_SetAudioDeviceGain(Channels[Channel_Music], Volume);
    SDL_SetAudioDeviceGain(Channels[Channel_SFX], Volume);
}

sound audio_system::CreateSound(const std::string &Filepath, audio_channel Channel)
{
    Log(Info, "audio_system::CreateSound()  - Creating sound %s, bound to Channel: %s", Filepath.c_str(), Channel == Channel_Music ? "Channel_Music" : "Channel_SFX");

    // Find a free asset slot
    sound_asset *Asset = nullptr;
    i16 AssetIndex = -1;
    for(u32 Index = 0; Index < MaxSoundAssetCount; ++Index)
    {
        if(Assets[Index].DataBuffer == nullptr)
        {
            Asset = &Assets[Index];
            AssetIndex = Index;
            break;
        }
    }

    // Found no free asset slot, return (-1, -1)
    if(Asset == nullptr)
        return InvalidHandle();

    // Found Asset Slot, load file
    bool Success = SDL_LoadWAV(Filepath.c_str(), &Asset->AudioSpec, &Asset->DataBuffer, &Asset->DataBufferLength);
    if(!Success)
    {
        Log(Error, "Failed to load wave file: %s, Error: %s", Filepath.c_str(), SDL_GetError());

        return InvalidHandle();
    }

    Asset->Filename = Filepath;
    Asset->Channel = Channel;
    Asset->Repeats = false;
    Asset->Volume = 0.5f;

    AssetsCount++;

    return CreateHandle(AssetIndex, Asset->Generation);
}

void audio_system::DestroySound(sound Handle)
{
    sound_asset *Asset = ResolveHandle(Handle);
    if(Asset == nullptr)
    {
        Log(Info,"audio_system::DestroySound - Tried to destroy an already destroyed asset");
        return;
    }

    Log(Info, "audio_system::DestroySound() - Destroying sound: %s", Asset->Filename.c_str());

    Asset->Generation++;
    Asset->Filename.clear();
    Asset->Channel = Channel_Invalid;
    Asset->Repeats = false;
    Asset->AudioSpec = {};
    Asset->DataBufferLength = 0;
    SDL_free(Asset->DataBuffer);
    Asset->DataBuffer = nullptr;

    // Clear all sound streams
    for(int i = 0; i < MaxConcurrentStreams; ++i)
    {
        SDL_DestroyAudioStream(Asset->Streams[i]);
        Asset->Streams[i] = nullptr;
    }

    AssetsCount--;
}

//
// Private
//

sound audio_system::CreateHandle(i16 Index, i16 Generation)
{
    return (Index << 16) | (Generation & 0xFFFF);
}

sound audio_system::InvalidHandle()
{
    return CreateHandle(-1, -1);
}

sound_asset *audio_system::ResolveHandle(sound Handle)
{
    i16 Index = Handle >> 16;
    i16 Generation = Handle & 0xFFFF;

    if(Index < 0 || Index > MaxSoundAssetCount - 1)
    {
        Log(Warning, "audio_system::ResolveHandle() - Tried to use invalid sound handle!");
        return nullptr;
    }

    if(Assets[Index].Generation != Generation)
    {
        Log(Warning, "audio_system::ResolveHandle() - Tried to use invalid sound handle!");
        return nullptr;
    }

    return &Assets[Index];
}

i16 audio_system::GetIndexFromHandle(sound Handle)
{
    return Handle >> 16;
}

i16 audio_system::GetGenerationFromHandle(sound Handle)
{
    return Handle & 0xFFFF;
}

void SDLCALL AudioStreamGetCallback(void *UserData, SDL_AudioStream *Stream, int AdditionalAmount, int TotalAmount)
{
    sound_asset *Asset = (sound_asset*)UserData;

    for(i32 i = 0; i < MaxConcurrentStreams; ++i)
    {
        if(Asset->Streams[i] != nullptr)
        {
            int SoundBytesRemaining = SDL_GetAudioStreamAvailable(Asset->Streams[i]);
            if(SoundBytesRemaining == 0)
            {
                SDL_PutAudioStreamData(Asset->Streams[i], Asset->DataBuffer, Asset->DataBufferLength);
            }
        }
    }
}
