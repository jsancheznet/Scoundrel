#ifdef VERTEX_SHADER

#extension GL_ARB_bindless_texture : require

layout(location = 0) in vec3 LocalVertexPosition;
layout(location = 1) in vec2 UV;
layout(location = 2) in vec3 InstanceWorldPosition;
layout(location = 3) in vec3 InstanceWorldScale;
layout(location = 4) in float InstanceWorldRotation;
layout(location = 5) in uvec2 TextureHandle;
layout(location = 6) in vec4 SrcRect;
layout(location = 7) in vec4 Tint;

layout (std140, binding = 50) uniform CameraMatrices
{
    mat4 Perspective;
    mat4 Orthographic;
    mat4 View;
};

uniform mat4 Model;

out vec2 Uv;
flat out uvec2 fsTextureHandle;
out vec4 TintColor;

mat4 CreateModelMatrix(vec3 Pos, vec3 Scale, float Rotation)
{
    float C = cos(Rotation);
    float S = sin(Rotation);

    return mat4
    (
        C * Scale.x, S * Scale.x, 0.0, 0.0,
        -S * Scale.y, C * Scale.y, 0.0, 0.0,
        0.0, 0.0, Scale.z, 0.0,
        Pos.x, Pos.y, Pos.z, 1.0
    );
}

void main()
{
    mat4 Model = CreateModelMatrix(InstanceWorldPosition, InstanceWorldScale, InstanceWorldRotation);
    gl_Position =  Perspective * View * Model * vec4(LocalVertexPosition, 1.0);
    Uv = mix(SrcRect.xy, SrcRect.zw, UV);
    fsTextureHandle = TextureHandle;
    TintColor = Tint;
}

#endif

#ifdef FRAGMENT_SHADER

#extension GL_ARB_bindless_texture : require

flat in uvec2 fsTextureHandle;
in vec2 Uv;
in vec4 TintColor;
out vec4 FragColor;

void main()
{
    vec4 TextureColor = texture(sampler2D(fsTextureHandle), Uv);
    FragColor = mix(TextureColor, TintColor, TintColor.a);
    // FragColor = (fsTextureHandle == uvec2(0u)) ? vec4(1,0,0,1)   // RED  = handle is zero
    //                                            : vec4(0,1,0,1);  // GREEN = handle arrived
}

#endif
