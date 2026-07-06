#version 330

in vec2 fragTexCoord;   // 0..1 across the output texture
in vec4 fragColor;

uniform float Scale;  
uniform float Octaves; 
uniform float Brightness;  
uniform float Seed;    

out vec4 finalColor;

// almpst random gradient direction per grid cell
vec2 gradient(vec2 cell)
{
    float angle = 43758.5453123 * sin(dot(cell, vec2(127.1, 311.7)));
    return vec2(cos(angle), sin(angle));
}

// perlin style gradient noise
float perlinNoise(vec2 p)
{
    vec2 cell = floor(p);
    vec2 f = fract(p);

    float a = dot(gradient(cell + vec2(0.0, 0.0)), f - vec2(0.0, 0.0));
    float b = dot(gradient(cell + vec2(1.0, 0.0)), f - vec2(1.0, 0.0));
    float c = dot(gradient(cell + vec2(0.0, 1.0)), f - vec2(0.0, 1.0));
    float d = dot(gradient(cell + vec2(1.0, 1.0)), f - vec2(1.0, 1.0));

    vec2 u = f * f * f * (f * (f * 6.0 - 15.0) + 10.0); // quintic smoothstep

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

// sum multiple octaves of noise at doubling frequency
float fbm(vec2 p, int octaves)
{
    float value = 0.0;
    float amplitude = 0.5;

    for (int i = 0; i < octaves; i++)
    {
        value += amplitude * perlinNoise(p);
        p *= 2.0;
        amplitude *= 0.5;
    }

    return value;
}

void main()
{
    vec2 p = fragTexCoord * Scale + vec2(Seed * 17.0, Seed * 31.0);

    int octaves = int(clamp(Octaves, 1.0, 8.0));
    float n = fbm(p, octaves);

    n = n * 0.5 + 0.5; // remap ~[-1,1] to [0,1]
    n *= Brightness;

    finalColor = vec4(vec3(n), 1.0);
}
