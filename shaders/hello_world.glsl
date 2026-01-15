#ifdef VERTEX_SHADER

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 UV;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec2 Uv;

void main()
{
    gl_Position =  Projection * View * Model * vec4(Position, 1.0);
    Uv = UV;
}

#endif

#ifdef FRAGMENT_SHADER

in vec2 Uv;
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.0, Uv.x, Uv.y, 1.0);
}

#endif
