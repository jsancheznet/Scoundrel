#include "renderer.h"

void renderer::Init()
{
    gladLoadGL();

    // TODO(Jsanchez): Enable debug mode
    // glEnable(GL_DEBUG_OUTPUT);
    // glDebugMessageCallback(DebugCallback, nullptr);

    // TODO(Jsanchez): Configure blend mode
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void renderer::ClearScreen(color Color)
{
    glClearColor(Color.r, Color.g, Color.b, Color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}
