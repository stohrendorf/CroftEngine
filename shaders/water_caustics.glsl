#include "time_uniform.glsl"
#include "noise.glsl"

vec3 snoise3(vec3 p)
{
    return vec3(
    snoise(p.xy),
    snoise(p.zx),
    snoise(p.yz)
    );
}

float voronoi(in vec3 p)
{
    vec3 i_st = floor(p);
    vec3 f_st = fract(p);

    float m_dist = 1.;// minimum distance

    for (int y= -1; y <= 1; y++) {
        float fy = float(y);
        for (int x= -1; x <= 1; x++) {
            float fx = float(x);
            for (int z= -1; z <= 1; z++) {
                // Neighbor place in the grid
                vec3 neighbor = vec3(fx, fy, float(z));

                // Random position from current + neighbor place in the grid
                vec3 point = snoise3(i_st + neighbor);

                // Animate the point
                point = sin(vec3(u_time*0.005) + 6.2831*point) * .5 + .5;

                // Vector between the sample and the point
                vec3 diff = neighbor + point - f_st;

                // Distance to the point
                float dist2 = dot(diff, diff);

                // Keep the closer distance
                m_dist = min(m_dist, dist2);
            }
        }
    }

    return m_dist;
}

float water_multiplier(in vec3 vpos)
{
    const float Scale1 = 0.003;
    float result = clamp(abs(voronoi(vpos * Scale1))+0.5, 0, 1);
    const float Scale2 = 0.0011;
    result *= clamp(abs(voronoi(vpos * Scale2))+0.5, 0, 1);
    return result;
}
