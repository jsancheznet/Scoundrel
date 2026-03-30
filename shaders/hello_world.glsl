#ifdef VERTEX_SHADER

layout (location = 0) in vec3 Position; // TODO(Jsanchez): Rename this variable to VertexLocalPosition
layout (location = 1) in vec2 UV;

layout(location = 2) in vec3 WorldPosition;
layout(location = 3) in vec3 Scale;
layout(location = 4) in float Rotation;

layout (std140, binding = 50) uniform CameraMatrices
{
    mat4 Perspective;
    mat4 Orthographic;
    mat4 View;
};

uniform mat4 Model;

out vec2 Uv;

void main()
{
    gl_Position =  Perspective * View * Model * vec4(Position, 1.0);
    Uv = UV;
}

#endif

#ifdef FRAGMENT_SHADER

uniform sampler2D Texture;

in vec2 Uv;
out vec4 FragColor;

void main()
{
    FragColor = texture(Texture, Uv);
}

#endif
