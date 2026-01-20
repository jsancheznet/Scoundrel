#pragma once

#include "typedefs.h"

struct mouse
{
    u32 ButtonState;
    u32 PrevButtonState;

    // These two coords start on the top left corner
    float X;
    float Y;

    void Update(); // This needs to be called after all events have been pumped

    b32 IsButtonPressed(u32 Button);
    b32 WasButtonPressed(u32 Button);
};
