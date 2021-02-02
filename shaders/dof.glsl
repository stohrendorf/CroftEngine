#ifdef WATER
float dof_start = 32.0/far_plane;
float dof_dist = 4*1024.0/far_plane;
float dof_focal_depth = 1536.0/far_plane;
#else
float dof_start = 128.0/far_plane;
float dof_dist = 20*1024.0/far_plane;
// autofocus
float dof_focal_depth = depth_at(vec2(0.5, 0.5));
#endif
const float dof_blur_range = 3;

#include "depth.glsl"

vec2 dof_texel = 1.0 / vec2(textureSize(u_texture, 0));

#include "util.glsl"

vec3 dof_color(in vec2 uv, in float blur_amount)//processing the sample
{
    const float fringe = 0.7;//bokeh chromatic aberration/fringing
    vec2 dr = vec2(0.0, 1.0)*dof_texel*fringe*blur_amount;
    vec2 dg = vec2(-0.866, -0.5)*dof_texel*fringe*blur_amount;
    vec2 db = vec2(0.866, -0.5)*dof_texel*fringe*blur_amount;

    vec3 col;
    col.r = shaded_texel(u_texture, uv+dr, depth_at(uv+dr)).r;
    col.g = shaded_texel(u_texture, uv+dg, depth_at(uv+dg)).g;
    col.b = shaded_texel(u_texture, uv+db, depth_at(uv+db)).b;

    const vec3 lumcoeff = vec3(0.299, 0.587, 0.114);
    const float threshold = 0.7;//highlight threshold;
    const float gain = 100.0;//highlight gain;

    float thresh = max((dot(col, lumcoeff)-threshold)*gain, 0.0);
    return col + col*thresh*blur_amount;
}

vec3 do_dof(in vec2 uv)
{
    float depth = depth_at(uv);
    float blur_amount = clamp((abs(depth-dof_focal_depth) - dof_start) / dof_dist, -dof_blur_range, dof_blur_range);

    const float namount = 0.0001;//dither amount
    vec2 noise = rand2(uv)*namount*blur_amount;

    const int samples = 3;//samples on the first ring
    const float bokeh_bias = 0.5;//bokeh edge bias
    vec2 blur_radius = dof_texel * blur_amount + noise;

    vec3 col = shaded_texel(u_texture, uv, depth_at(uv));
    float weight_sum = 1.0;

    const int rings = 3;
    for (int i = 1; i <= rings; ++i)
    {
        int ringsamples = i * samples;

        for (int j = 0; j < ringsamples; ++j)
        {
            float angle = float(j) * PI * 2.0 / float(ringsamples);
            vec2 dxy = vec2(cos(angle), sin(angle)) * float(i);
            float weight = mix(1.0, float(i)/float(rings), bokeh_bias);
            col += dof_color(uv + dxy*blur_radius, blur_amount) * weight;
            weight_sum += weight;
        }
    }
    col /= weight_sum;
    return col;
}
