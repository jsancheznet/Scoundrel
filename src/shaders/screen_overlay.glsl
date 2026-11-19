#ifdef VERTEX_SHADER

layout(location = 0) in vec3 LocalVertexPosition;

void main()
{
    // The quad goes from -0.5 to 0.5, so twice its size fills the screen. No camera, the camera shake must not move it
    gl_Position = vec4(LocalVertexPosition.xy * 2.0, 0.0, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

uniform vec4 Color;

out vec4 FragmentColor;

void main()
{
    FragmentColor = Color;
}

#endif
