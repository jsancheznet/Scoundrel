#pragma once

#include "typedefs.h"
#include "audio.h"
#include "renderer.h"
#include "bitmap_font.h"

constexpr u32 InvalidIndex = UINT32_MAX;

constexpr u32 MaxTextureCount = 128;
constexpr u32 MaxSoundCount   = 32;
constexpr u32 MaxFontCount    = 4;

// Fixed u8 underlying type so any byte read from a handle (e.g. 0xFF from InvalidHandle) is a valid asset_type value
enum asset_type : u8
{
    Asset_None = 0,
    Asset_Sound = 1,
    Asset_Texture = 2,
    Asset_BitmapFont = 3,

    Asset_Count
};

// Bookkeeping stored at the start of every asset slot
struct asset_header
{
    b32 Used;
    u32 Generation;
    char Name[64];
};

struct texture_asset { asset_header Header; texture Texture; };
struct sound_asset   { asset_header Header; sound Sound; };
struct font_asset    { asset_header Header; bitmap_font Font; };

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

    // Return nullptr for invalid, stale or wrong type handles
    texture     *GetTexture(u64 Handle);
    sound       *GetSound(u64 Handle);
    bitmap_font *GetFont(u64 Handle);

private:

    texture_asset Textures[MaxTextureCount];
    sound_asset   Sounds[MaxSoundCount];
    font_asset    Fonts[MaxFontCount];

    // Handle layout: [Type 8 bits][Index 24 bits][Generation 32 bits]
    inline u64 CreateHandle(asset_type Type, u32 Index, u32 Generation);
    inline u64 InvalidHandle();
    inline asset_type GetTypeFromHandle(u64 Handle);
    inline u32 GetIndexFromHandle(u64 Handle);
    inline u32 GetGenerationFromHandle(u64 Handle);

    b32 IsValid(u64 Handle, asset_type Type, asset_header *Header);
};
