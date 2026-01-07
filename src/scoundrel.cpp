#include "typedefs.h"
#include "application.h"
#include "renderer.h"

#include <stdio.h>

#include <SDL3/SDL.h>

application Application;
renderer    Renderer;

i32 main(i32 Argc, char **Argv)
{
    // TODO(Jsanchez): Render two triangles
    // TODO(Jsanchez): Create simple logging system, logs to a file and stdout
    // TODO(Jsanchez): Windowed fullscreen toggle on ALT+ENTER
    // TODO(Jsanchez): Import Miniaud.io, play SFX and song

    Application.Init();
    Application.CreateWindow("Scoundrel", 1366, 768);

    Renderer.Init(Application.Window);

    //
    // Estudiar RendererCreateBuffer, anotar que hace cada funcion y para que sirve, agregar por parametro ese 3
    // Despues ver si vale la pena dejar el bindeo ahi en el buffer, yo creo que no, pero bueno, tal vez se bindea antes de dibujar cada uno
    // Podria probar creando otro buffer con un quad y dibujar los dos
    //

    //
    // 1- Crear abstraccion para dibujar el triangulo asi como esta
    // 2- Actualizar esa abstraccion para ponerle colores a los vertices del triangulo
    // 3- ACtualizar esa abstraccion para dibujar un QUAD
    // 4- Cargar una textura
    // 5- Dibujar un Quad texturizado, usar directamente las cartas que descargue
    // 6- Dibujar varios quads texturizados
    // 7- Meter dos cartas dentro de un buffer con sus posiciones y usar una sola draw call
    //

    u32 HelloWorldShader = Renderer.CompileShader("shaders/hello_world.glsl");

    f32 TriangleVertices[] =
    {
        0.0f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f
    };

    u32 TriangleBuffer = Renderer.CreateBuffer(sizeof(TriangleVertices), sizeof(f32) * 3, TriangleVertices);

    Renderer.UseShader(HelloWorldShader);

    while(Application.IsRunning)
    {
        Application.ProcessEvents();

        Renderer.ClearScreen(ORANGE);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        Renderer.EndFrame();
    }

    Application.Quit();
}
