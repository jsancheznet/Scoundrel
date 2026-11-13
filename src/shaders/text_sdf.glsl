#ifdef VERTEX_SHADER

#extension GL_ARB_bindless_texture : require

// Local vertex data in NDC coords
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 UV;

// Instance Data per sprite
layout(location = 2) in vec3 Position; // Pixels for screen text, world units for world text
layout(location = 3) in vec4 Rect; // UV rect in the texture atlas (left, bottom, right, top)
layout(location = 4) in vec2 Size;
layout(location = 5) in vec4 Color;
layout(location = 6) in uvec2 TextureHandle;
layout(location = 7) in int WorldSpace;

layout (std140, binding = 50) uniform CameraMatrices
{
    mat4 Perspective;
    mat4 Orthographic;
    mat4 View;
};

out vec2 TexCoord;
flat out uvec2 fsTextureHandle;
flat out vec4 fsColor;

void main()
{
    vec3 Corner = vec3(Position.xy + UV * Size, Position.z);
    if(WorldSpace != 0)
    {
        gl_Position = Perspective * View * vec4(Corner, 1.0);
    }
    else
    {
        gl_Position = Orthographic * vec4(Corner, 1.0);
        gl_Position.z = -gl_Position.w; // Screen text on the near plane, always passes the depth test
    }
    // Flip V: the atlas is loaded flipped vertically and the ortho projection has Y pointing down
    TexCoord = mix(Rect.xy, Rect.zw, vec2(UV.x, 1.0 - UV.y));
    fsTextureHandle = TextureHandle;
    fsColor = Color;
}

#endif

#ifdef FRAGMENT_SHADER

#extension GL_ARB_bindless_texture : require

in vec2 TexCoord;
flat in uvec2 fsTextureHandle;
flat in vec4 fsColor;

out vec4 FragmentColor;

void main()
{
    float Distance = texture(sampler2D(fsTextureHandle), TexCoord).r;
    float Smoothing = fwidth(Distance); // Keeps the edge ~1px wide at any size
    float Alpha = smoothstep(0.5 - Smoothing, 0.5 + Smoothing, Distance);
    FragmentColor = vec4(fsColor.rgb, fsColor.a * Alpha);
}

#endif
