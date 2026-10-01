#pragma once

#include <string>

#include "typedefs.h"
#include "audio.h"
#include "renderer.h"
#include "bitmap_font.h"

constexpr u32 InvalidIndex = UINT32_MAX;
constexpr u32 InvalidGeneration = UINT32_MAX;
constexpr u32 MaxAssetCount = 64;

enum asset_type
{
    Asset_None = 0,
    Asset_Sound = 1,
    Asset_Texture = 2,
    Asset_BitmapFont = 3,

    Asset_Count
};

struct asset
{
    asset_type Type;
    b32 Used;
    u32 Generation;
    char Name[256];

    union
    {
        sound Sound;
        texture Texture;
        bitmap_font Font;
    };
};

struct asset_manager
{
    u32 AssetCount = 0;

    void Init();

    // Asset Loading
    u64 LoadSound(const char *Filepath, audio_channel Channel);
    u64 LoadTexture(const char *Filepath);
    u64 LoadBitmapFont(const char *Image, const char *Json);

    // Asset Unloading
    b32 Unload(u64 Handle);

    // Handle stuff
    void *ResolveHandle(u64 Handle);

private:

    asset Assets[MaxAssetCount];

    inline u64 CreateHandle(u32 Index, u32 Generation);
    inline u64 InvalidHandle();
    b32 IsValid(u64 Handle);
    inline u32 GetIndexFromHandle(u64 Handle);
    inline u32 GetGenerationFromHandle(u64 Handle);

    u32 FindFreeIndex();
};
