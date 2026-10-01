#pragma once

// The following is the command used to generate the bitmap font used in this project.
//     msdf-gen command -> ~/Projects/Scoundrel/tools/msdf-atlas-gen -font MarcellusSC-Regular.ttf -charset charset.txt -fontname marcellus_sc -type sdf -format png -pots -imageout marcellus_sc.png -csv marcellus_sc.csv -json marcellus_sc.json -minsize 42

#include "typedefs.h"
#include "renderer.h"

struct font_atlas
{
    char Type[16];
    i32 DistanceRange;
    i32 DistanceRangeMiddle;
    i32 Width;
    i32 Height;
    f32 Size;
};

struct font_metrics
{
    i32 EmSize;
    f32 LineHeight;
    f32 Ascender;
    f32 Descender;
    f32 UnderlineY;
    f32 Thickness;
};

struct font_glyph_bounds
{
    f32 Left;
    f32 Right;
    f32 Bottom;
    f32 Top;
};

struct font_glyph
{
    i32 UnicodeId;
    f32 Advance;

    font_glyph_bounds PlaneBounds;
    font_glyph_bounds AtlasBounds;
};

constexpr i32 MaxGlyphCount = 96;
constexpr i32 MaxCodepoint  = 128;
constexpr u8  InvalidGlyph  = 0xFF;

struct bitmap_font
{
    texture Texture;

    char Name[64];
    font_atlas Atlas;
    font_metrics Metrics;


    i32 GlyphCount;
    u8 GlyphLookupTable[MaxCodepoint]; // Index this array by character to get the Index into the Glyphs array, Index =
                                       // GlyphLookupTable['A']
    font_glyph Glyphs[MaxGlyphCount];
};

font_glyph *GetGlyph(bitmap_font *Font, char Character);
