#include "asset_manager.h"
#include "typedefs.h"
#include "log.h"
#include "helpers.h"
#include "renderer.h"
#include "json.h"

#include <cstring>

#ifndef STB_IMAGE_IMPLEMENTATION
    #define STB_IMAGE_IMPLEMENTATION
    #include <stb_image.h>
#endif

extern renderer Renderer;

// Fills Bounds from a json object with "left", "right", "bottom" and "top", NULL leaves them zeroed
static void JsonGetBounds(json_object_s *Object, font_glyph_bounds *Bounds)
{
    Bounds->Left   = (f32)JsonGetNumber(Object, "left");
    Bounds->Right  = (f32)JsonGetNumber(Object, "right");
    Bounds->Bottom = (f32)JsonGetNumber(Object, "bottom");
    Bounds->Top    = (f32)JsonGetNumber(Object, "top");
}

// Frees the GPU texture and the CPU image data
static void ReleaseTexture(texture *Texture)
{
    Renderer.DeleteTexture(&Texture->ID, &Texture->BindlessTextureHandle);
    Texture->Width = 0;
    Texture->Height = 0;
    Texture->ChannelCount = 0;
    stbi_image_free(Texture->Data);
    Texture->Data = nullptr;
}

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
    std::strncpy(Asset->Name, Filename, std::strlen(Filename));

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

u64 asset_manager::LoadTexture(const char *Filepath)
{
    Log(Info, "asset_manager::LoadTexture(%s)", Filepath);

    if(!FileExists(Filepath))
    {
        Log(Error, "asset_manager::LoadTexture(%s) - File does not exist", Filepath);
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

    // Set Asset Vars
    Asset->Type = Asset_Texture;
    Asset->Used = true;
    const char *Filename = FilenameFromPath(Filepath); Assert(Filename);
    std::strncpy(Asset->Name, Filename, std::strlen(Filename));

    // Set Texture Vars
    stbi_set_flip_vertically_on_load(true);
    Asset->Texture.Data = stbi_load(Filepath, &Asset->Texture.Width, &Asset->Texture.Height, &Asset->Texture.ChannelCount, 4);

    Renderer.UploadTexture(Asset->Texture.Data,
                           Asset->Texture.Width,
                           Asset->Texture.Height,
                           &Asset->Texture.ID,
                           &Asset->Texture.BindlessTextureHandle);

    AssetCount++;

    return CreateHandle(Index, Asset->Generation);
}

u64 asset_manager::LoadBitmapFont(const char *Image, const char *Json)
{
    if(!FileExists(Image) || !FileExists(Json))
    {
        Log(Error, "asset_manager::LoadBitmapFont(%s, %s) - One of the files does not exist!", Image, Json);
        return InvalidHandle();
    }

    u32 Index = FindFreeIndex();
    if(Index == InvalidIndex)
    {
        Log(Warning, "asset_manager::LoadBitmapFont(%s, %s) - Found no free slot inside the asset_manager", Image, Json);
        return InvalidHandle();
    }

    size_t JsonSize;
    char *JsonContents = (char*)SDL_LoadFile(Json, &JsonSize);
    json_value_s *JsonRoot = JsonContents ? json_parse(JsonContents, JsonSize) : NULL;
    SDL_free(JsonContents);

    json_object_s *RootObject = JsonRoot ? json_value_as_object(JsonRoot) : NULL;
    json_object_s *AtlasObject = JsonGetObject(RootObject, "atlas");
    if(!AtlasObject)
    {
        Log(Error, "asset_manager::LoadBitmapFont(%s, %s) - Invalid json or missing \"atlas\" object", Image, Json);
        free(JsonRoot);
        return InvalidHandle();
    }

    // Load the atlas image before claiming the slot, so a bad image doesn't leave a half loaded font.
    // msdf-atlas-gen writes atlasBounds with yOrigin "bottom", which matches flipping on load
    texture Texture = {};
    stbi_set_flip_vertically_on_load(true);
    Texture.Data = stbi_load(Image, &Texture.Width, &Texture.Height, &Texture.ChannelCount, 4);
    if(!Texture.Data)
    {
        Log(Error, "asset_manager::LoadBitmapFont(%s, %s) - Failed to load image: %s", Image, Json, stbi_failure_reason());
        free(JsonRoot);
        return InvalidHandle();
    }

    // Fill the Asset Data
    asset *Asset = &Assets[Index];
    Asset->Type = Asset_BitmapFont;
    Asset->Used = true;
    const char *Name = FilenameFromPath(Image); Assert(Name);
    std::snprintf(Asset->Name, sizeof(Asset->Name), "%s", Name);

    // Fill the font data
    bitmap_font *Font = &Asset->Font;
    Font->Texture = Texture;
    Renderer.UploadTexture(Font->Texture.Data, Font->Texture.Width, Font->Texture.Height, &Font->Texture.ID, &Font->Texture.BindlessTextureHandle);

    const char *FontName = JsonGetString(RootObject, "name");
    std::snprintf(Font->Name, sizeof(Font->Name), "%s", FontName ? FontName : "");

    // Font Atlas
    font_atlas *Atlas = &Font->Atlas;
    const char *AtlasType = JsonGetString(AtlasObject, "type");
    std::snprintf(Atlas->Type, sizeof(Atlas->Type), "%s", AtlasType ? AtlasType : "");
    Atlas->DistanceRange       = (i32)JsonGetNumber(AtlasObject, "distanceRange");
    Atlas->DistanceRangeMiddle = (i32)JsonGetNumber(AtlasObject, "distanceRangeMiddle");
    Atlas->Width               = (i32)JsonGetNumber(AtlasObject, "width");
    Atlas->Height              = (i32)JsonGetNumber(AtlasObject, "height");
    Atlas->Size                = (f32)JsonGetNumber(AtlasObject, "size");

    // Font Metrics
    json_object_s *MetricsObject = JsonGetObject(RootObject, "metrics");
    if(!MetricsObject)
    {
        Log(Warning, "asset_manager::LoadBitmapFont(%s, %s) - Missing \"metrics\" object", Image, Json);
    }

    font_metrics *Metrics = &Font->Metrics;
    Metrics->EmSize     = (i32)JsonGetNumber(MetricsObject, "emSize");
    Metrics->LineHeight = (f32)JsonGetNumber(MetricsObject, "lineHeight");
    Metrics->Ascender   = (f32)JsonGetNumber(MetricsObject, "ascender");
    Metrics->Descender  = (f32)JsonGetNumber(MetricsObject, "descender");
    Metrics->UnderlineY = (f32)JsonGetNumber(MetricsObject, "underlineY");
    Metrics->Thickness  = (f32)JsonGetNumber(MetricsObject, "underlineThickness");

    // Glyphs
    Font->GlyphCount = 0;
    std::memset(Font->GlyphLookupTable, InvalidGlyph, sizeof(Font->GlyphLookupTable));

    json_array_s *GlyphsArray = JsonGetArray(RootObject, "glyphs");
    if(!GlyphsArray)
    {
        Log(Warning, "asset_manager::LoadBitmapFont(%s, %s) - Missing \"glyphs\" array", Image, Json);
    }

    // Loop over all the glyphs, extract the data.
    for(json_array_element_s *Element = GlyphsArray ? GlyphsArray->start : NULL; Element; Element = Element->next)
    {
        json_object_s *GlyphObject = json_value_as_object(Element->value);
        if(!GlyphObject) continue;

        i32 UnicodeId = (i32)JsonGetNumber(GlyphObject, "unicode", -1);
        if(UnicodeId < 0 || UnicodeId >= MaxCodepoint)
        {
            Log(Warning, "asset_manager::LoadBitmapFont(%s, %s) - Glyph %d is outside the supported codepoint range, skipping", Image, Json, UnicodeId);
            continue;
        }

        if(Font->GlyphCount >= MaxGlyphCount)
        {
            Log(Warning, "asset_manager::LoadBitmapFont(%s, %s) - Reached MaxGlyphCount (%d), skipping the remaining glyphs", Image, Json, MaxGlyphCount);
            break;
        }

        Font->GlyphLookupTable[UnicodeId] = (u8)Font->GlyphCount;
        font_glyph *Glyph = &Font->Glyphs[Font->GlyphCount++];
        Glyph->UnicodeId = UnicodeId;
        Glyph->Advance   = (f32)JsonGetNumber(GlyphObject, "advance");

        // Whitespace glyphs (like space) have no bounds, they end up zeroed
        JsonGetBounds(JsonGetObject(GlyphObject, "planeBounds"), &Glyph->PlaneBounds);
        JsonGetBounds(JsonGetObject(GlyphObject, "atlasBounds"), &Glyph->AtlasBounds);
    }

    json_array_s *KerningArray = JsonGetArray(RootObject, "kerning");
    if(KerningArray && KerningArray->length > 0)
    {
        Log(Warning, "asset_manager::LoadBitmapFont(%s, %s) - Font has %zu kerning pairs, kerning is not implemented", Image, Json, KerningArray->length);
    }

    free(JsonRoot); // json.h allocates the whole tree in a single block

    AssetCount++;

    return CreateHandle(Index, Asset->Generation);
}

b32 asset_manager::Unload(u64 Handle)
{
    u32 Index = GetIndexFromHandle(Handle);
    u32 Generation = GetGenerationFromHandle(Handle);

    // Reject out of range, already free, or stale handles (slot reused by another asset)
    if(Index >= MaxAssetCount ||
       !Assets[Index].Used ||
       Assets[Index].Generation != Generation)
    {
        Log(Warning, "asset_manager::Unload() - Tried to unload an invalid or stale asset handle");
        return false;
    }

    asset *Asset = &Assets[Index];

    Log(Info, "asset_manager::Unload(%s)", Asset->Name);

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
        case Asset_Texture:
        {
            ReleaseTexture(&Asset->Texture);
            break;
        }
        case Asset_BitmapFont:
        {
            ReleaseTexture(&Asset->Font.Texture);
            std::memset(&Asset->Font, 0, sizeof(bitmap_font));
            break;
        }
        default:
        {
            Log(Warning, "asset_manager::Unload() - Invalid Asset Type");
            break;
        }
    }

    Asset->Used = false;
    std::memset(Asset->Name, 0, sizeof(Asset->Name));
    Asset->Type = Asset_None;
    Asset->Generation++;
    AssetCount--;

    return true;
}

void *asset_manager::ResolveHandle(u64 Handle)
{
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
