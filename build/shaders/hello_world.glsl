#ifdef VERTEX_SHADER

layout (location = 0) in vec3 Position;

void main()
{
    gl_Position = vec4(Position.x, Position.y, Position.z, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}

#endif
