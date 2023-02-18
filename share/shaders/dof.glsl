#include "camera_interface.glsl"

#ifdef IN_WATER
float dof_start = 32.0 * InvFarPlane;
float dof_dist = 4 * 1024.0 * InvFarPlane;
float dof_focal_depth = 1536.0 * InvFarPlane;
#else
float dof_start = 128.0 * InvFarPlane;
float dof_dist = 20*1024.0 * InvFarPlane;
// autofocus
float dof_focal_depth = -texture(u_geometryPosition, vec2(0.5)).z * InvFarPlane;
#endif
const float DofBlurRange = 3;

vec2 dof_texel = 1.0 / vec2(textureSize(u_texture, 0));

vec3 do_dof(in vec2 uv)
{
    float depth = -texture(u_geometryPosition, uv).z * InvFarPlane;
    float blur_amount = clamp((abs(depth-dof_focal_depth) - dof_start) / dof_dist, -DofBlurRange, DofBlurRange);

    const float NAmount = 0.001;//dither amount
    vec2 noise = noise2(uv) * NAmount * blur_amount;

    const int Samples = 3;//samples on the first ring
    const float BokehBias = 0.5;//bokeh edge bias
    vec2 blur_radius = dof_texel * blur_amount + noise;

    vec3 col = texture(u_texture, uv).rgb;
    float weight_sum = 1.0;

    const int Rings = 3;
    const float BokehDelta = 1.0 / Rings;
    float bokehFactor = 0;
    for (int i = 1; i <= Rings; ++i)
    {
        int ringsamples = int(sqrt(i) * Samples);
        float angleDelta = PI * 2.0 / float(ringsamples);

        float angle = 0;
        for (int j = 0; j < ringsamples; ++j)
        {
            vec2 dxy = vec2(cos(angle), sin(angle)) * float(i);
            float weight = mix(1.0, bokehFactor, BokehBias);
            vec2 sampleUv = dxy*blur_radius + uv;
            col = texture(u_texture, sampleUv).rgb * weight + col;
            weight_sum += weight;
            angle += angleDelta;
        }

        bokehFactor += BokehDelta;
    }
    return col / weight_sum;
}
