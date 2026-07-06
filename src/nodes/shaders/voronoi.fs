#version 330


// SOURCE: https://github.com/tuxalin/procedural-tileable-shaders/blob/master/voronoi.glsl

// i also lwk had to use claude for modifying this cause i don't really know much about shader programming n shi

in vec2 fragTexCoord;
in vec4 fragColor;

uniform vec2 Pos;
uniform vec2 Scale;
uniform float Jitter;
uniform float Phase;
uniform float Seed;

out vec4 finalColor;


vec2 multiHash2D(vec2 p)
{
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453123) * 2.0 - 1.0;
}

vec4 multiHash2D(vec2 p1, vec2 p2)
{
    return vec4(multiHash2D(p1), multiHash2D(p2));
}

float hash1D(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

vec3 hash3D(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * vec3(0.1031, 0.1030, 0.0973));
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.xxy + p3.yzz) * p3.zyx);
}

// gradient noise with analytic derivative, IQ-style: returns (value, dValue/dx, dValue/dy)
vec3 gradientNoised(vec2 pos, vec2 scale, float phase, float seed)
{
    pos *= scale;
    vec2 i = floor(pos);
    vec2 f = fract(pos);

    vec2 u = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);
    vec2 du = 30.0 * f * f * (f * (f - 2.0) + 1.0);

    vec2 ga = multiHash2D(mod(i + vec2(0.0, 0.0), scale) + seed);
    vec2 gb = multiHash2D(mod(i + vec2(1.0, 0.0), scale) + seed);
    vec2 gc = multiHash2D(mod(i + vec2(0.0, 1.0), scale) + seed);
    vec2 gd = multiHash2D(mod(i + vec2(1.0, 1.0), scale) + seed);

    float va = dot(ga, f - vec2(0.0, 0.0));
    float vb = dot(gb, f - vec2(1.0, 0.0));
    float vc = dot(gc, f - vec2(0.0, 1.0));
    float vd = dot(gd, f - vec2(1.0, 1.0));

    float value = va + u.x * (vb - va) + u.y * (vc - va) + u.x * u.y * (va - vb - vc + vd);
    vec2 derivative = ga + u.x * (gb - ga) + u.y * (gc - ga) + u.x * u.y * (ga - gb - gc + gd)
                     + du * (u.yx * (va - vb - vc + vd) + vec2(vb, vc) - va);

    return vec3(value, derivative);
}

vec3 voronoi(vec2 pos, vec2 scale, float jitter, float phase, float seed)
{
     // voronoi based on Inigo Quilez: https://archive.is/Ta7dm
    const float kPI2 = 6.2831853071;
    pos *= scale;
    vec2 i = floor(pos);
    vec2 f = pos - i;

    // first pass
    vec2 minPos, tilePos;
    float minDistance = 1e+5;
    for (int k=0; k<8; k+=2)
    {
        ivec2 k1 = ivec2(k, k + 1);
        ivec2 ky = k1 / 3;
        vec4 n = vec4(k1 - ky * 3, ky).xzyw - 1.0;
        
        vec4 ni = mod(i.xyxy + n, scale.xyxy) + seed;
        vec4 cPos = multiHash2D(ni.xy, ni.zw) * jitter;
        cPos = 0.5 * sin(phase + kPI2 * cPos) + 0.5;
        vec4 rPos = n + cPos - f.xyxy;

        vec4 temp = rPos * rPos;
        temp.xy = temp.xz + temp.yw;
        vec4 minResult = temp.x < temp.y ? vec4(rPos.xy, cPos.xy) : vec4(rPos.zw, cPos.zw);
        float d = min(temp.x, temp.y);
        if(d < minDistance)
        {
            minDistance = d;
            minPos = minResult.xy;
            tilePos = minResult.zw;
        }
    }
    // last cell
    {
        vec2 n = vec2(1.0);
        vec2 ni = mod(i.xy + n, scale) + seed;
        vec2 cPos = multiHash2D(ni) * jitter;
        cPos = 0.5 * sin(phase + kPI2 * cPos) + 0.5;
        vec2 rPos = n + cPos - f;

        float d = dot(rPos, rPos);
        if(d < minDistance)
        {
            minDistance = d;
            minPos = rPos;
            tilePos = cPos;
        }
    }

    // second pass, distance to edges
    minDistance = 1e+5;
    for (int y=-2; y<=2; y++)
    {
        for (int x=-2; x<=2; x+=2)
        { 
            vec4 n = vec4(x, y, x + 1, y);
            vec4 ni = mod(i.xyxy + n, scale.xyxy) + seed;
            vec4 cPos = multiHash2D(ni.xy, ni.zw) * jitter;
            cPos = 0.5 * sin(phase + kPI2 * cPos) + 0.5;
            vec4 rPos = n + cPos - f.xyxy;
            
            // compute the perpendicular distance
            vec4 temp = minPos.xyxy - rPos;
            temp *= temp;
            vec2 l  = temp.xz + temp.yw;
            vec4 a = 0.5 * (minPos.xyxy + rPos);
            vec4 b = rPos - minPos.xyxy;
            temp = b * b;
            b /= sqrt(temp.xz + temp.yw).xxyy;
            
            temp = a * b;
            vec2 d = temp.xz + temp.yw; 
            if(l.x > 1e-5) 
                minDistance = min(minDistance, d.x);
            if(l.y > 1e-5) 
                minDistance = min(minDistance, d.y);
                
        }      
    }

    return vec3(minDistance, tilePos);
}

// Voronoi with the position and minimum distance.
// @param scale Number of tiles, must be  integer for tileable results, range: [2, inf]
// @param jitter Jitter factor for the voronoi cells, if zero then it will result in a square grid, range: [0, 1], default: 1.0
// @param phase The phase for rotating the cells, range: [0, inf], default: 0.0
// @param seed Seed to randomize result, range: [0, inf], default: 0.0
// @return Returns the distance from the cell edges, yz = tile position of the cell, range: [0, 1]
vec3 voronoiPosition(vec2 pos, vec2 scale, float jitter, float phase, float seed)
{
    const float kPI2 = 6.2831853071;
    pos *= scale;
    vec2 i = floor(pos);
    vec2 f = pos - i;

    // first pass
    vec2 tilePos;
    float minDistance = 1e+5;
    for (int k=0; k<8; k+=2)
    {
        ivec2 k1 = ivec2(k, k + 1);
        ivec2 ky = k1 / 3;
        vec4 n = vec4(k1 - ky * 3, ky).xzyw - 1.0;
        
        vec4 ni = mod(i.xyxy + n, scale.xyxy) + seed;
        vec4 cPos = multiHash2D(ni.xy, ni.zw) * jitter;
        cPos = 0.5 * sin(phase + kPI2 * cPos) + 0.5;
        vec4 rPos = n + cPos - f.xyxy;

        vec4 temp = rPos * rPos;
        temp.xy = temp.xz + temp.yw;

        vec3 minResult = temp.x < temp.y ? vec3(cPos.xy, temp.x) : vec3(cPos.zw, temp.y);
        float d = minResult.z;
        if(d < minDistance)
        {
            minDistance = d;
            tilePos = minResult.xy;
        }
    }
    // last cell
    {
        vec2 n = vec2(1.0);
        vec2 ni = mod(i.xy + n, scale) + seed;
        vec2 cPos = multiHash2D(ni) * jitter;
        cPos = 0.5 * sin(phase + kPI2 * cPos) + 0.5;
        vec2 rPos = n + cPos - f;

        float d = dot(rPos, rPos);
        if(d < minDistance)
        {
            minDistance = d;
            tilePos = cPos;
        }
    }
    return vec3(tilePos, minDistance);
}

// @param scale Number of tiles, must be  integer for tileable results, range: [2, inf]
// @param jitter Jitter factor for the voronoi cells, if zero then it will result in a square grid, range: [0, 1], default: 1.0
// @param variance The color variance, if zero then it will result in grayscale pattern, range: [0, 1], default: 1.0
// @param factor The position factor multiplier, range: [0, 10], default: 1.0
// @param seed Random seed for the color pattern, range: [0, inf], default: 0.0
// @return Returns the color of the pattern cells., range: [0, 1]
vec3 voronoiPattern(vec2 pos, vec2 scale, float jitter, float variance, float factor, float seed)
{
    vec2 tilePos = voronoiPosition(pos, scale, jitter, 0.0, 0.0).xy;
    float rand = abs(hash1D(tilePos * factor + seed));
    return (rand < variance ? hash3D(tilePos + seed) : vec3(rand));
}

// @param scale Number of tiles, must be  integer for tileable results, range: [2, inf]
// @param jitter Jitter factor for the voronoi cells, if zero then it will result in a square grid, range: [0, 1], default: 1.0
// @param width Width of the lines, range: [0, 1], default: 0.1
// @param smoothness Controls how soft the lines are, range: [0, 1], default: 0.0
// @param warp The warp strength, range: [0, 1], default: 0.0
// @param warpScale The scale of warp, range: [0, 1], default: 2.0
// @param warpSmudge If true creates a smudge effect on the lines, range: [false, true], default: false
// @param seed Seed to randomize result, range: [0, inf], default: 0.0
// @return Returns the cell position and the value of the pattern, range: [0, 1]
vec3 cracks(vec2 pos, vec2 scale, float jitter, float width, float smoothness, float warp, float warpScale, bool warpSmudge, float smudgePhase, float seed)
{
    vec3 g = gradientNoised(pos, scale * warpScale, smudgePhase, seed);
    pos += (warpSmudge ? g.yz : g.xx) * 0.1 * warp;
    vec3 v = voronoi(pos, scale, jitter, 0.0, seed);
    return vec3(smoothstep(max(width - smoothness, 0.0), width + fwidth(v.x), v.x), v.yz);
}

// @param scale Number of tiles, must be  integer for tileable results, range: [2, inf]
// @param jitter Jitter factor for the voronoi cells, if zero then it will result in a square grid, range: [0, 1], default: 1.0
// @param width Width of the lines, range: [0, 1], default: 0.1
// @param smoothness Controls how soft the lines are, range: [0, 1], default: 0.0
// @param warp The warp strength, range: [0, 1], default: 0.0
// @param warpScale The scale of warp, range: [0, 1], default: 2.0
// @param warpSmudge If true creates a smudge effect on the lines, range: [false, true], default: false
// @param seed Seed to randomize result, range: [0, inf], default: 0.0
// @return Returns the value of the pattern, range: [0, 1]
float cracks(vec2 pos, vec2 scale, float jitter, float width, float smoothness, float warp, float warpScale, bool warpSmudge, float seed)
{   
    return cracks(pos, scale, jitter, width, smoothness, warp, warpScale, warpSmudge, 0.0, seed).x;
}

void main(){
finalColor = vec4(voronoi(fragTexCoord + Pos,Scale,Jitter,Phase,Seed), 1.0);
}