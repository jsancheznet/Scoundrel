#ifdef VERTEX_SHADER

layout(location = 0) in vec3 LocalVertexPosition;
layout(location = 1) in vec2 UV;

out vec2 Uv;

void main()
{
    // The quad goes from -0.5 to 0.5, so twice its size fills the screen. No camera, the camera shake must not move it
    gl_Position = vec4(LocalVertexPosition.xy * 2.0, 0.0, 1.0);
    Uv = UV;
}

#endif

#ifdef FRAGMENT_SHADER

const vec3 DeepColor  = vec3(0.020, 0.025, 0.070);
const vec3 MistColor  = vec3(0.160, 0.090, 0.300);
const vec3 GlowColor  = vec3(0.300, 0.320, 0.620);
const float MistScale = 2.2;   // Larger = smaller patches of mist
const float DriftSpeed = 0.035; // Larger = faster mist
const float Brightness = 1.0;
const float VignetteStrength = 0.55; // 0 = no dark edges

uniform float Time; // Seconds
uniform vec2 Resolution; // Pixels

in vec2 Uv;
out vec4 FragmentColor;

float Hash(vec2 P)
{
    return fract(sin(dot(P, vec2(127.1, 311.7))) * 43758.5453);
}

float Noise(vec2 P)
{
    vec2 Cell = floor(P);
    vec2 F = fract(P);
    vec2 Smooth = F * F * (3.0 - 2.0 * F);

    float A = Hash(Cell);
    float B = Hash(Cell + vec2(1.0, 0.0));
    float C = Hash(Cell + vec2(0.0, 1.0));
    float D = Hash(Cell + vec2(1.0, 1.0));

    return mix(mix(A, B, Smooth.x), mix(C, D, Smooth.x), Smooth.y);
}

float Fbm(vec2 P)
{
    // The rotation between octaves hides the grid of the value noise
    const mat2 Rotation = mat2(0.8, 0.6, -0.6, 0.8);

    float Sum = 0.0;
    float Amplitude = 0.5;
    for(int Octave = 0; Octave < 5; ++Octave)
    {
        Sum += Amplitude * Noise(P);
        P = Rotation * P * 2.0;
        Amplitude *= 0.5;
    }

    return Sum;
}

void main()
{
    // Y is the reference, so the mist keeps its shape when the window width changes
    vec2 Centered = Uv - 0.5;
    vec2 P = vec2(Centered.x * Resolution.x / Resolution.y, Centered.y) * MistScale;

    float T = Time * DriftSpeed;

    // Domain warp: the coordinates of the last noise are moved by two other noises, which gives the curl of mist
    vec2 Warp1 = vec2(Fbm(P + vec2(0.0, T)), Fbm(P + vec2(5.2, 1.3) - vec2(T, 0.0)));
    vec2 Warp2 = vec2(Fbm(P + 3.0 * Warp1 + vec2(1.7, 9.2) + 0.6 * T), Fbm(P + 3.0 * Warp1 + vec2(8.3, 2.8) - 0.5 * T));
    float Mist = Fbm(P + 3.0 * Warp2);

    vec3 Color = mix(DeepColor, MistColor, smoothstep(0.15, 0.85, Mist));
    Color = mix(Color, GlowColor, smoothstep(0.55, 1.0, Mist) * length(Warp2) * 0.45);

    float Vignette = 1.0 - VignetteStrength * smoothstep(0.25, 0.95, length(Centered) * 1.4);
    Color *= Vignette * Brightness;

    // Less than one color step of noise, it hides the bands that dark gradients show on a 8 bit screen
    Color += (Hash(gl_FragCoord.xy + fract(Time)) - 0.5) / 255.0;

    FragmentColor = vec4(Color, 1.0);
}

#endif
