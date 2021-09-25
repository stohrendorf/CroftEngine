#include "time_uniform.glsl"
#include "camera_interface.glsl"

float texel_shade(in float depth)
{
    return clamp(1.0 - depth * InvFarPlane, 0.0, 1.0);
}

vec3 shade_texel(in vec3 rgb, in float depth)
{
    return rgb * texel_shade(depth);
}

vec3 shaded_texel(in sampler2D tex, in vec2 uv, in float depth)
{
    return shade_texel(texture(tex, uv).rgb, depth);
}

float luminance(in vec3 color)
{
    return dot(color, vec3(0.212656, 0.715158, 0.072186));
}

float luminance(in vec4 color)
{
    return dot(vec3(color), vec3(0.212656, 0.715158, 0.072186));
}
