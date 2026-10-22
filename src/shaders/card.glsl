#ifdef VERTEX_SHADER

#extension GL_ARB_bindless_texture : require

// Per vertex data, shared by every card
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 Uv;

// Per instance data, one per card
layout(location = 3) in mat4 Model; // Uses locations 3, 4, 5 and 6
layout(location = 7) in uvec2 FrontTexture;
layout(location = 8) in uvec2 BackTexture;

layout (std140, binding = 50) uniform CameraMatrices
{
    mat4 Perspective;
    mat4 Orthographic;
    mat4 View;
};

flat out uvec2 fsFrontTextureHandle;
flat out uvec2 fsBackTextureHandle;
flat out float fsSide;
out vec2 TexCoords;

void main()
{
    gl_Position = Perspective * View * Model * vec4(Position, 1.0);

    fsFrontTextureHandle = FrontTexture;
    fsBackTextureHandle = BackTexture;
    TexCoords = Uv;

    // Model space normal: +1 on the front, -1 on the back, 0 on the edges
    fsSide = Normal.z;
}

#endif

#ifdef FRAGMENT_SHADER

#extension GL_ARB_bindless_texture : require

flat in uvec2 fsFrontTextureHandle;
flat in uvec2 fsBackTextureHandle;
flat in float fsSide;
in vec2 TexCoords;

out vec4 FragColor;

void main()
{
    // NOTE: gl_FrontFacing can't pick the side, each side is its own outward facing triangles,
    // so whichever side faces the camera is "front facing". The model space normal tells them apart.
    uvec2 Handle = (fsSide < -0.5) ? fsBackTextureHandle : fsFrontTextureHandle;
    FragColor = texture(sampler2D(Handle), TexCoords);
}

#endif
