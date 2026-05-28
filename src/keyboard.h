#pragma once

#include "typedefs.h"

struct keyboard
{
    bool *CurrentState;
    bool *PreviousState;

    i32 NumKeys;
    i32 NumKeysAllocSize;

    void Init();
    void Update();

    bool IsPressed(SDL_Scancode Scancode);
    bool IsReleased(SDL_Scancode Scancode);
};
