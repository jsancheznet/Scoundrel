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


void audio_system::Play(sound Sound)
{
    Log(Info, "audio::system::Play() - Playing sound %s", Sound.Name.c_str());
    SDL_BindAudioStream(Channels[Sound.Channel], Sound.Stream);
}

void audio_system::Pause(sound Sound)
{
    Log(Info, "audio_system::Pause() - Pausing sound %s", Sound.Name.c_str());
    SDL_UnbindAudioStream(Sound.Stream);
}

void audio_system::SetSoundVolume(sound Sound, f32 Volume)
{
    Volume = glm::clamp(0.0f, 1.0f, Volume);

    Log(Info, "audio_system::SetSoundVolume - Sound: %s, Volume: %f", Sound.Name.c_str(), Volume);

    SDL_SetAudioStreamGain(Sound.Stream, Volume);
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

sound audio_system::CreateSound(const std::string &Path, audio_channel Channel, bool Repeats)
{
    sound Result = {};

    Log(Info, "audio_system::CreateSound()  - Creating sound %s, bound to Channel: %s", Path.c_str(), Channel == Channel_Music ? "Channel_Music" : "Channel_SFX");

    bool Success = SDL_LoadWAV(Path.c_str(), &Result.Spec, &Result.Buffer, &Result.Length);
    if(!Success)
    {
        Log(Error, "Failed to load wave file: %s, Error: %s", Path.c_str(), SDL_GetError());
        return Result;
    }

    SDL_AudioSpec DeviceSpec;
    SDL_GetAudioDeviceFormat(Channels[Channel], &DeviceSpec, NULL);

    Result.Name = Path;
    Result.Repeats = Repeats;
    Result.Stream = SDL_CreateAudioStream(&Result.Spec, &DeviceSpec);
    Result.Channel = Channel;

    SDL_PutAudioStreamData(Result.Stream, Result.Buffer, Result.Length);

#if 0
    if(Repeats)
    {
        b32 Success = SDL_SetAudioStreamGetCallback(Result.Stream, AudioStreamGetCallback, &Result);
    }
#endif

    return Result;
}

void audio_system::DestroySound(sound &Sound)
{
    if(Sound.Stream == nullptr || Sound.Buffer == nullptr || Sound.Length == 0 || Sound.Channel == Channel_Invalid)
    {
        Log(Warning, "audio_system::DestroySound() - Trying to Destroy an already destroyed sound");
        return;
    }

    Log(Info, "audio_system::DestroySound() - Destroying sound: %s", Sound.Name.c_str());

    Sound.Name.clear();

    SDL_DestroyAudioStream(Sound.Stream);
    Sound.Stream = nullptr;

    SDL_free(Sound.Buffer);
    Sound.Buffer = 0;
    Sound.Length = 0;

    Sound.Channel = Channel_Invalid;
}

#if 0
void SDLCALL AudioStreamGetCallback(void *UserData, SDL_AudioStream *Stream, int AdditionalAmount, int TotalAmount)
{
    sound *Sound = (sound*)UserData;

    // Copy more data for sounds that are supposed to repeat
    int SoundBytesRemaining = SDL_GetAudioStreamAvailable(Stream);
    if(SoundBytesRemaining == 0)
    {
        SDL_PutAudioStreamData(Stream, Sound->Buffer, Sound->Length);
    }
}
#endif
