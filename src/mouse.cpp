#include "typedefs.h"
#include "mouse.h"
#include "log.h"

#include <SDL3/SDL.h>

void mouse::Update()
{
    PrevButtonState = ButtonState;
    // NOTE: SDL_GetRelativeMouseState,returns x,y accumulated since last call
    ButtonState = SDL_GetMouseState(&X, &Y);
}

b32 mouse::IsButtonPressed(u32 Button)
{
    return ButtonState & SDL_BUTTON_MASK(Button);
}

b32 mouse::WasButtonPressed(u32 Button)
{
    return PrevButtonState & SDL_BUTTON_MASK(Button);
}
