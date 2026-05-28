#include "keyboard.h"
#include "typedefs.h"
#include "log.h"

#include <SDL3/SDL.h>

void keyboard::Init()
{
    const bool* SDLKeyboardState = SDL_GetKeyboardState(&NumKeys);
    NumKeysAllocSize = sizeof(bool) * NumKeys;

    CurrentState = (bool*)SDL_malloc(NumKeysAllocSize);
    PreviousState = (bool*)SDL_malloc(NumKeysAllocSize);
    Log(Info, "keyboard::Init - Allocated %d bytes for Previous && Current state", NumKeysAllocSize);

    SDL_memcpy(CurrentState, SDLKeyboardState, NumKeysAllocSize);
    SDL_memcpy(PreviousState, SDLKeyboardState, NumKeysAllocSize);
}

void keyboard::Update()
{
    SDL_memcpy(PreviousState, CurrentState, NumKeysAllocSize);

    const bool* SDLKeyboardState = SDL_GetKeyboardState(nullptr);

    SDL_memcpy(CurrentState, SDLKeyboardState, NumKeysAllocSize);
}

bool keyboard::IsPressed(SDL_Scancode Scancode)
{
    return CurrentState[Scancode];
}

bool keyboard::IsReleased(SDL_Scancode Scancode)
{
    return !CurrentState[Scancode] && PreviousState[Scancode];
}
