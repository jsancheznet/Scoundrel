#ifdef VERTEX_SHADER

#extension GL_ARB_bindless_texture : require

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 Uv;

// TODO: Add Instance Data

layout (std140, binding = 50) uniform CameraMatrices
{
    mat4 Perspective;
    mat4 Orthographic;
    mat4 View;
};

void main()
{
    float s = 0.01; // half size
    mat4 Model = mat4(
        s,   0.0, 0.0, 0.0,
        0.0, s,   0.0, 0.0,
        0.0, 0.0, s,   0.0,
        0.0, 0.0, 0.0, 1.0
    );
    gl_Position = Perspective * View * Model * vec4(Position, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

#extension GL_ARB_bindless_texture : require

out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}

#endif