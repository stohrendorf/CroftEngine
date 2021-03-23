#include "time_uniform.glsl"

float cellnoise(in vec3 p)
{
    return fract(sin(dot(p, vec3(12.9898, 78.233, 54.849))) * 43758.5453) * 2 - 1;
}

vec3 cellnoise3(vec3 p)
{
    return vec3(
    cellnoise(p.xyz),
    cellnoise(p.zxy),
    cellnoise(p.yzx)
    );
}

float voronoi(in vec3 p)
{
    vec3 i_st = floor(p);
    vec3 f_st = fract(p);

    float m_dist = 1.;// minimum distance

    for (int y= -1; y <= 1; y++) {
        for (int x= -1; x <= 1; x++) {
            for (int z= -1; z <= 1; z++) {
                // Neighbor place in the grid
                vec3 neighbor = vec3(float(x), float(y), float(z));

                // Random position from current + neighbor place in the grid
                vec3 point = cellnoise3(i_st + neighbor);

                // Animate the point
                point = 0.5 + 0.5*sin(u_time*0.005 + 6.2831*point);

                // Vector between the pixel and the point
                vec3 diff = neighbor + point - f_st;

                // Distance to the point
                float dist = length(diff);

                // Keep the closer distance
                m_dist = min(m_dist, dist);
            }
        }
    }

    return pow(m_dist, 2);
}

float water_multiplier(in vec3 vpos)
{
    const float Scale1 = 0.003;
    float result = clamp(abs(voronoi(vpos * Scale1))+0.5, 0, 1);
    const float Scale2 = 0.0011;
    result *= clamp(abs(voronoi(vpos * Scale2))+0.5, 0, 1);
    return result;
}
