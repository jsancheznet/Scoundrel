#pragma once

#include <SDL3/SDL.h>
#include <glad/glad.h>

#include "typedefs.h"

#define HOT_PINK color{1.0f, 0.0f, 1.0f, 0.0f}
#define ORANGE   color{1.0f, 0.647f, 0.0f, 0.0f}

struct color
{
    f32 r, g, b, a;
};

struct renderer
{
    SDL_Window* Window;

    u32 MainVAO;

    void Init(SDL_Window *SDLWindow);

    u32 CompileShader(const char *Filename);
    u32 CreateBuffer(u32 Size, u32 Stride, void *Data);

    void UseShader(u32 Shader);
    void ClearScreen(color Color);
    void EndFrame();


private:

    static void DebugCallback(GLenum Source, GLenum Type, GLuint Id,  GLenum Severity, GLsizei Length, GLchar const *Message, void const *UserParam);
};
