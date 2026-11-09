#include "bitmap_font.h"
#include "typedefs.h"

font_glyph *GetGlyph(bitmap_font *Font, char Character)
{
    u8 Codepoint = (u8)Character;
    if(Codepoint >= MaxCodepoint) return NULL;

    u8 Index = Font->GlyphLookupTable[Codepoint];
    if(Index == InvalidGlyph) return NULL;

    return &Font->Glyphs[Index];
}

// Width of the widest line of Text at font size Size (em size), same pen advance as renderer::DrawText
f32 TextWidth(bitmap_font *Font, f32 Size, const char *Text)
{
    if(!Font || !Text) return 0.0f;

    f32 MaxWidth = 0.0f;
    f32 PenX = 0.0f;

    for(const char *C = Text; *C; ++C)
    {
        if(*C == '\n')
        {
            if(PenX > MaxWidth) MaxWidth = PenX;
            PenX = 0.0f;
            continue;
        }

        font_glyph *Glyph = GetGlyph(Font, *C);
        if(Glyph == nullptr)
        {
            // Fonts generated without a space glyph still need the gap between words
            if(*C == ' ') PenX += 0.25f * Size;
            continue;
        }

        PenX += Glyph->Advance * Size;
    }

    if(PenX > MaxWidth) MaxWidth = PenX;
    return MaxWidth;
}
