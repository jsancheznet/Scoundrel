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
