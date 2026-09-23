#include "asset_manager.h"
#include "typedefs.h"
#include "log.h"
#include "helpers.h"

#include <cstring>

void asset_manager::Init()
{
    Log(Info, "asset_manager::Init()");

    // Zero the asset array
    memset(Assets, 0, sizeof(asset) * MaxAssetCount);

    return;
}

u64 asset_manager::LoadSound(const char *Filepath, audio_channel Channel)
{
    Log(Info, "asset_manager::LoadSound(%s)", Filepath);

    if(!FileExists(Filepath))
    {
        Log(Error, "asset_manager::LoadSound(%s) - File does not exist!", Filepath);
        return InvalidHandle();
    }

    // Find a free slot, return if nothing was found
    u32 Index = FindFreeIndex();
    if(Index == InvalidIndex)
    {
        Log(Warning, "asset_manager::LoadSound(%s) - Found no free slot inside the asset_manager", Filepath);
        return InvalidHandle();
    }

    asset *Asset = &Assets[Index];

    // Fill Asset related Data
    Asset->Type = Asset_Sound;
    Asset->Used = true;
    const char *Filename = FilenameFromPath(Filepath); Assert(Filename);
    std::strncpy(Asset->Filename, Filename, std::strlen(Filename));

    // Fill Sound Related Data
    {
        // Load Wave file
        bool Success = SDL_LoadWAV(Filepath, &Asset->Sound.AudioSpec, &Asset->Sound.DataBuffer, &Asset->Sound.DataBufferLength);
        if(!Success)
        {
            Log(Error, "asset_manager::LoadSound() - Failed to load wave file: %s, Error: %s", Filepath, SDL_GetError());
            return InvalidHandle();
        }

        Asset->Sound.Channel = Channel;
        Asset->Sound.Repeats = false;
    }

    AssetCount++;

    return CreateHandle(Index, Asset->Generation);

}

b32 asset_manager::Unload(u64 Handle)
{
    u32 Index = GetIndexFromHandle(Handle);

    if(Index == InvalidIndex)
    {
        Log(Warning, "asset_manager::Unload() - Tried to unload an invalid asset handle");
        return false;
    }

    asset *Asset = &Assets[Index];

    Log(Info, "asset_manager::Unload(%s)", Asset->Filename);

    Asset->Used = false;
    std::memset(Asset->Filename, 0, sizeof(Asset->Filename));

    switch(Asset->Type)
    {
        case Asset_Sound:
        {
            Asset->Sound.AudioSpec = {};
            Asset->Sound.Channel = Channel_None;
            Asset->Sound.DataBufferLength = 0;
            SDL_free(Asset->Sound.DataBuffer);
            Asset->Sound.DataBuffer = nullptr;
            for(int i = 0; i < MaxConcurrentStreams; ++i)
            {
                SDL_DestroyAudioStream(Asset->Sound.Streams[i]);
                Asset->Sound.Streams[i] = nullptr;
            }

            break;
        }
        default:
        {
            Log(Warning, "asset_manager::Unload() - Invalid Asset Type");
            break;
        }
    }

    Asset->Type = Asset_None;
    Asset->Generation++;
    AssetCount--;

    return true;
}

void *asset_manager::ResolveHandle(u64 Handle)
{
    Log(Info, "asset_manager::ResolveHandle()");

    u32 Index = GetIndexFromHandle(Handle);
    u32 Generation = GetGenerationFromHandle(Handle);

    if(Index == InvalidIndex ||
       Generation == InvalidGeneration ||
       Assets[Index].Generation != Generation)
    {
        Log(Warning, "asset_manager::ResolveHandle() - Trying to resolve invalid asset handle");
        return nullptr;
    }

    return &Assets[Index].Sound; // Address of union storage
}

u64 asset_manager::CreateHandle(u32 Index, u32 Generation)
{
    return ((u64)Index << 32) | ((u64)Generation);
}

u64 asset_manager::InvalidHandle()
{
    return ((u64)UINT32_MAX << 32) | ((u64)UINT32_MAX);
}

b32 asset_manager::IsValid(u64 Handle)
{
    u32 Index = GetIndexFromHandle(Handle);
    u32 Gen = GetGenerationFromHandle(Handle);

    if(Index != UINT32_MAX && Gen != UINT32_MAX)
    {
        return true;
    }

    return false;
}

u32 asset_manager::GetIndexFromHandle(u64 Handle)
{
    return Handle >> 32;
}

u32 asset_manager::GetGenerationFromHandle(u64 Handle)
{
    return Handle & 0xFFFFFFFF;
}

u32 asset_manager::FindFreeIndex()
{
    for(u32 i = 0; i < MaxAssetCount; ++i)
    {
        if(!Assets[i].Used)
        {
            return i;
        }
    }

    return InvalidIndex;
}
