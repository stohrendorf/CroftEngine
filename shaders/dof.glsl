#ifdef WATER
const float dof_scale = 200.0 / Z_max;
const float dof_offset = 80.0 / Z_max;
#else
const float dof_scale = 40.0 / Z_max;
const float dof_offset = 20.0 / Z_max;
#endif

#include "depth.glsl"

// autofocus
float dof_focal_depth = depth_at(vec2(0.5, 0.5));

#include "util.glsl"

float dof_focal_point_distance(in float depth)
{
    return clamp(abs(depth - dof_focal_depth) / dof_focal_depth, 0.0, 1.0);
}

float dof_blur_radius(in float depth) {
    return dof_focal_point_distance(depth) * dof_scale + dof_offset;
}

vec3 do_dof(in vec2 uv, in vec3 centerRgb)
{
    float depth = depth_at(uv);

    const int dof_blends = 6;
    const int dof_rings = 6;
    const float angle_step = PI*2.0 / dof_blends;

    float dist_step = dof_blur_radius(depth) / dof_rings;

    vec3 sample_color = centerRgb;
    float sample_weight_sum = 1.0;
    for (int i = 1; i <= dof_blends; i += 1)
    {
        for (int j = 0; j < dof_rings; j += 1)
        {
            vec2 p;
            p.x = sin(i*angle_step) * dist_step * j;
            p.y = cos(i*angle_step) * dist_step * j;
            vec2 r = rand2(uv + p) * dist_step / 2.0;
            vec2 peek = uv + p + r;
            float local_weight = 1.0 - smoothstep(j + 1.0, 0.0, dof_rings);
            sample_color += shaded_texel(peek, depth_at(peek)) * local_weight;
            sample_weight_sum += local_weight;
        }
    }
    sample_color /= sample_weight_sum;
    return sample_color;
}
