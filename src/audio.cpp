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

    SDL_GetAudioDeviceFormat(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &DeviceSpec, NULL);
    DeviceID = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &DeviceSpec);
}

void audio_system::Play(sound Sound)
{
    Log(Info, "audio::system::Play(Sound)");
}

void audio_system::Pause()
{
    Log(Info, "AudioSystem::Pause(%d)", DeviceID);

    SDL_PauseAudioDevice(DeviceID);
}

void audio_system::Resume()
{
    Log(Info, "AudioSystem::ResumePlayback(%d)", DeviceID);

    SDL_ResumeAudioDevice(DeviceID);
}

void audio_system::SetVolume(f32 Volume)
{
    Log(Info, "AudioSystem::SetVolume(%.2f)", Volume);

    SDL_SetAudioDeviceGain(DeviceID, Volume);
}

sound audio_system::CreateSound(const char* Path)
{
    sound Result = {};

    Log(Info, "CreateSound(%s)", Path);

    bool Success = SDL_LoadWAV(Path, &Result.Spec, &Result.Buffer, &Result.Length);
    if(!Success)
    {
        Log(Error, "Failed to load wave file: %s, Error: %s", Path, SDL_GetError());
        return Result;
    }

    Channel.Stream = SDL_CreateAudioStream(&Result.Spec, &DeviceSpec);
    SDL_BindAudioStream(DeviceID, Channel.Stream);
    SDL_PutAudioStreamData(Channel.Stream, Result.Buffer, Result.Length);

    return Result;
}
