#include "time_uniform.glsl"
#include "noise.glsl"

float voronoi(in vec3 p)
{
    vec3 i_st = floor(p);
    vec3 f_st = fract(p);

    float m_dist = 1.;// minimum distance
    const float Epsilon = 1.0/255.0;
    const float Offset = 0.5;

    for (float fy = -1; fy <= 1; fy += 1.0) {
        for (float fx = -1; fx <= 1; fx += 1.0) {
            for (float fz = -1; fz <= 1; fz += 1.0) {
                vec3 neighbor = vec3(fx, fy, fz);
                // Random position from current + neighbor place in the grid
                vec3 point = snoise3(i_st + neighbor);

                // Animate the point
                point = sin(vec3(u_time*0.005) + 6.2831*point) * .5 + .5;

                // Vector between the sample and the point
                vec3 diff = neighbor + point - f_st;
                float dist = dot(diff, diff);
                if (dist < Epsilon) {
                    return Offset;
                }
                m_dist = min(m_dist, dist);
            }
        }
    }

    return m_dist + Offset;
}

float water_multiplier(in vec3 vpos)
{
    const float Scale1 = 0.003;
    const float Scale2 = 0.0011;
    float result = clamp(voronoi(vpos * Scale1) * voronoi(vpos * Scale2), 0, 1);
    return result;
}
