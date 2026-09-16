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
    PauseAll();
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
    Volume = glm::clamp(0.0f, 1.0f, Volume);

    Log(Info, "audio_system::SetChannelVolume - Channel: %d, Volume: %.2f", Channel, Volume);

    SDL_SetAudioDeviceGain(Channels[Channel], Volume);
}

void audio_system::Play(sound Handle)
{
    sound_slot *Slot = ResolveHandle(Handle);
    if(Slot == nullptr)
        return;

    Log(Info, "audio::system::Play() - Playing sound %s", Slot->Filename.c_str());
    SDL_BindAudioStream(Channels[Slot->Channel], Slot->Stream);
}

void audio_system::Pause(sound Handle)
{
    sound_slot *Slot = ResolveHandle(Handle);
    if(Slot == nullptr)
        return;

    Log(Info, "audio_system::Pause() - Pausing sound %s", Slot->Filename.c_str());
    SDL_UnbindAudioStream(Slot->Stream);
}

void audio_system::SetSoundVolume(sound Handle, f32 Volume)
{
    sound_slot *Slot = ResolveHandle(Handle);
    if(Slot == nullptr)
        return;

    Volume = glm::clamp(0.0f, 1.0f, Volume);

    Log(Info, "audio_system::SetSoundVolume - Sound: %s, Volume: %f", Slot->Filename.c_str(), Volume);

    SDL_SetAudioStreamGain(Slot->Stream, Volume);
}

void audio_system::SetRepeat(sound Handle, b32 ShouldRepeat)
{
    sound_slot *Slot = ResolveHandle(Handle);
    if(Slot == nullptr)
        return;

    if(ShouldRepeat)
    {
        // Assign the callback that copies the audio again once it finishes
        SDL_SetAudioStreamGetCallback(Slot->Stream, AudioStreamGetCallback, Slot);
    }
    else
    {
        // Turn off the callback
        SDL_SetAudioStreamGetCallback(Slot->Stream, NULL, Slot);
    }
}

void audio_system::PauseAll()
{
    Log(Info, "AudioSystem::PauseAll - Pausing All Sound Devices");
    SDL_PauseAudioDevice(Channels[Channel_Music]);
    SDL_PauseAudioDevice(Channels[Channel_SFX]);
}

void audio_system::ResumeAll()
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

sound audio_system::CreateSound(const std::string &Path, audio_channel Channel)
{
    Log(Info, "audio_system::CreateSound()  - Creating sound %s, bound to Channel: %s", Path.c_str(), Channel == Channel_Music ? "Channel_Music" : "Channel_SFX");

    // Look for a free slot
    for(u32 Index = 0; Index < MaxSoundCount; ++Index)
    {
        sound_slot *Slot = &Sounds[Index];

        if(Slot->InUse == false)
        {
            bool Success = SDL_LoadWAV(Path.c_str(), &Slot->Spec, &Slot->AudioDataBuffer, &Slot->AudioDataBufferLength);
            if(!Success)
            {
                Log(Error, "Failed to load wave file: %s, Error: %s", Path.c_str(), SDL_GetError());

                return InvalidHandle();
            }

            SDL_AudioSpec DeviceSpec;
            SDL_GetAudioDeviceFormat(Channels[Channel], &DeviceSpec, NULL);

            Slot->InUse = true;
            Slot->Filename = Path;
            Slot->Stream = SDL_CreateAudioStream(&Slot->Spec, &DeviceSpec);
            Slot->Channel = Channel;

            SDL_PutAudioStreamData(Slot->Stream, Slot->AudioDataBuffer, Slot->AudioDataBufferLength);

            return CreateHandle(Index, Slot->Generation);
        }
    }

    return InvalidHandle();
}

void audio_system::DestroySound(sound Handle)
{
    sound_slot *Slot = ResolveHandle(Handle);
    if(Slot == nullptr)
        return;

    Log(Info, "audio_system::DestroySound() - Destroying sound: %s", Slot->Filename.c_str());

    Slot->InUse = false;
    Slot->Generation++;
    Slot->Filename.clear();
    Slot->Repeats = false;
    Slot->Channel = Channel_Invalid;
    Slot->Spec = {};
    SDL_DestroyAudioStream(Slot->Stream);
    Slot->Stream = nullptr;
    SDL_free(Slot->AudioDataBuffer);
    Slot->AudioDataBuffer = 0;
    Slot->AudioDataBufferLength = 0;
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

sound_slot *audio_system::ResolveHandle(sound Handle)
{
    i16 Index = Handle >> 16;
    i16 Generation = Handle & 0xFFFF;

    if(Index < 0 || Index > MaxSoundCount - 1)
    {
        Log(Warning, "audio_system::ResolveHandle() - Tried to use invalid sound handle!");
        return nullptr;
    }

    if(Sounds[Index].Generation != Generation)
    {
        Log(Warning, "audio_system::ResolveHandle() - Tried to use invalid sound handle!");
        return nullptr;
    }

    return &Sounds[Index];
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
    sound_slot *Slot = (sound_slot*)UserData;

    // Copy more data for sounds that are supposed to repeat
    int SoundBytesRemaining = SDL_GetAudioStreamAvailable(Stream);
    if(SoundBytesRemaining == 0)
    {
        SDL_PutAudioStreamData(Stream, Slot->AudioDataBuffer, Slot->AudioDataBufferLength);
    }
}
