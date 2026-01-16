#pragma once

#include "typedefs.h"

struct texture
{
    u32 Id;
    i32 Width;
    i32 Height;
};

texture CreateTexture(const char *File);
