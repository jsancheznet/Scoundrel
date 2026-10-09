#ifdef VERTEX_SHADER

#extension GL_ARB_bindless_texture : require

// Local vertex data in NDC coords
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 UV;

// Instance Data per sprite
layout(location = 2) in vec2 ScreenPosition;
layout(location = 3) in vec2 Rect; // UV coords for the texture atlas
layout(location = 4) in vec2 Size;
layout(location = 5) in vec3 Color;
layout(location = 6) in uvec2 TextureHandle;

layout (std140, binding = 50) uniform CameraMatrices
{
    mat4 Perspective;
    mat4 Orthographic;
    mat4 View;
};

out vec2 TexCoord;

void main()
{
    vec2 Corner = ScreenPosition + UV * Size;
    gl_Position = Orthographic * vec4(Corner, 0.0, 1.0);
    TexCoord = vec2(UV.x, UV.y);
}

#endif

#ifdef FRAGMENT_SHADER

in vec2 TexCoord;

out vec4 FragmentColor;
uniform sampler2D Texture;

void main()
{
    // float Smoothing = 0.02;
    // float Distance = texture(Texture, TexCoord).x;
    // float Alpha = smoothstep(0.5 - Smoothing, 0.5 + Smoothing, Distance);
    // FragmentColor = vec4(vec3(1.0), Alpha);

    FragmentColor = vec4(1.0, 0.0, 1.0, 1.0);

}

#endif
