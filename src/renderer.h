#pragma once

#include <glad/glad.h>
#include "typedefs.h"

#define HOT_PINK color{1.0f, 0.0f, 1.0f, 0.0f}
#define ORANGE color{1.0f, 0.647f, 0.0f, 0.0f}

struct color
{
    f32 r, g, b, a;
};

struct renderer
{
    void Init();

    void ClearScreen(color Color);
};
