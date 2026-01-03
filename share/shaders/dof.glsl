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

vec3 do_dof(in vec2 uv, in float geomDepth)
{
    float depth = geomDepth * InvFarPlane;
    float blur_amount = clamp((abs(depth-dof_focal_depth) - dof_start) / dof_dist, -DofBlurRange, DofBlurRange);

    const float NAmount = 0.001;//dither amount
    vec2 noise = noise2(uv) * NAmount * blur_amount;

    vec2 blur_radius = dof_texel * blur_amount + noise;

    vec3 col = texture(u_texture, uv).rgb;
    float weight_sum = 1.0;

    const vec2 samples[12] = vec2[](
    vec2(1.000000, 0.000000), vec2(-0.500000, 0.866025), vec2(-0.500000, -0.866025),
    vec2(2.000000, 0.000000), vec2(0.000000, 2.000000), vec2(-2.000000, 0.000000), vec2(0.000000, -2.000000),
    vec2(3.000000, 0.000000), vec2(0.927051, 2.853170), vec2(-2.427051, 1.763356), vec2(-2.427051, -1.763356), vec2(0.927051, -2.853170)
    );
    const float sampleWeights[12] = float[](
    0.500000, 0.500000, 0.500000,
    0.666667, 0.666667, 0.666667, 0.666667,
    0.833333, 0.833333, 0.833333, 0.833333, 0.833333
    );

    for (int i = 0; i < 12; ++i)
    {
        col += texture(u_texture, samples[i] * blur_radius + uv).rgb * sampleWeights[i];
        weight_sum += sampleWeights[i];
    }

    return col / weight_sum;
}
