#ifdef WATER
float dof_start = 32.0/u_farPlane;
float dof_dist = 4*1024.0/u_farPlane;
float dof_focal_depth = 1536.0/u_farPlane;
#else
float dof_start = 128.0/u_farPlane;
float dof_dist = 20*1024.0/u_farPlane;
// autofocus
float dof_focal_depth = texture(u_linearDepth, vec2(0.5)).r;
#endif
const float DofBlurRange = 3;

vec2 dof_texel = 1.0 / vec2(textureSize(u_texture, 0));

#include "util.glsl"

vec3 dof_color(in vec2 uv, in float blur_amount)//processing the sample
{
    const float fringe = 0.7;//bokeh chromatic aberration/fringing
    vec2 dr = vec2(0.0, 1.0)*dof_texel*fringe*blur_amount;
    vec2 dg = vec2(-0.866, -0.5)*dof_texel*fringe*blur_amount;
    vec2 db = vec2(0.866, -0.5)*dof_texel*fringe*blur_amount;

    vec3 col;
    col.r = shaded_texel(u_texture, uv+dr, texture(u_linearDepth, uv+dr).r).r;
    col.g = shaded_texel(u_texture, uv+dg, texture(u_linearDepth, uv+dg).r).g;
    col.b = shaded_texel(u_texture, uv+db, texture(u_linearDepth, uv+db).r).b;
    return col;
}

vec3 do_dof(in vec2 uv)
{
    float depth = texture(u_linearDepth, uv).r;
    float blur_amount = clamp((abs(depth-dof_focal_depth) - dof_start) / dof_dist, -DofBlurRange, DofBlurRange);

    const float NAmount = 0.0001;//dither amount
    vec2 noise = rand2(uv) * NAmount * blur_amount;

    const int Samples = 3;//samples on the first ring
    const float BokehBias = 0.5;//bokeh edge bias
    vec2 blur_radius = dof_texel * blur_amount + noise;

    vec3 col = shaded_texel(u_texture, uv, texture(u_linearDepth, uv).r);
    float weight_sum = 1.0;

    const int Rings = 3;
    const float BokehDelta = 1.0 / Rings;
    float bokehFactor = 0;
    for (int i = 1; i <= Rings; ++i)
    {
        int ringsamples = i * Samples;
        float angleDelta = PI * 2.0 / float(ringsamples);

        float angle = 0;
        for (int j = 0; j < ringsamples; ++j)
        {
            vec2 dxy = vec2(cos(angle), sin(angle)) * float(i);
            float weight = mix(1.0, bokehFactor, BokehBias);
            col += dof_color(dxy*blur_radius + uv, blur_amount) * weight;
            weight_sum += weight;
            angle += angleDelta;
        }

        bokehFactor += BokehDelta;
    }
    col /= weight_sum;
    return col;
}
