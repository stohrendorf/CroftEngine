#include "time_uniform.glsl"
#include "noise.glsl"

float voronoi(in vec3 p)
{
    vec3 i_st = floor(p);
    vec3 f_st = fract(p);

    const float Offset = 0.5;
    float m_dist = 1.0 - Offset;
    float time_val = TimeSeconds * 5.0;

    vec3 start = step(0.5, f_st) - 1.0;

    for (int y = 0; y <= 1; y++) {
        for (int x = 0; x <= 1; x++) {
            for (int z = 0; z <= 1; z++) {
                vec3 neighbor = start + vec3(float(x), float(y), float(z));
                vec3 point = snoise3(i_st + neighbor);
                point = sin(point * 6.2831853 + time_val) * 0.5 + 0.5;
                vec3 diff = neighbor + point - f_st;
                m_dist = min(m_dist, dot(diff, diff));
            }
        }
    }

    return m_dist + Offset;
}

float water_multiplier(in vec3 vpos)
{
    const float Scale1 = 0.003;
    const float Scale2 = 0.0011;
    float phase0 = snoise3(vpos*10).x;
    float s = sin(phase0 + TimeSeconds * 0.5);
    s = (1-s*s*0.75) + 0.25;
    return voronoi(vpos * Scale1) * voronoi(vpos * Scale2) * s;
}
