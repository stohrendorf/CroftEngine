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

// https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_sRGB.txt
float toSrgb(float cl)
{
    if (cl < 0.0031308) {
        return 12.92 * cl;
    }
    else {
        return 1.055 * pow(cl, 0.41666) - 0.055;
    }
}

vec3 toSrgb(vec3 cl)
{
    return vec3(
    toSrgb(cl.r),
    toSrgb(cl.g),
    toSrgb(cl.b)
    );
}

vec4 toSrgb(vec4 cl)
{
    return vec4(toSrgb(cl.rgb), cl.a);
}

// https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_sRGB_decode.txt
float toLinear(float cs)
{
    if (cs <= 0.04045) {
        return cs / 12.92;
    }
    else {
        return pow(((cs + 0.055)/1.055), 2.4);
    }
}

vec3 toLinear(vec3 cs) {
    return vec3(
    toLinear(cs.r),
    toLinear(cs.g),
    toLinear(cs.b)
    );
}

vec4 toLinear(vec4 cs)
{
    return vec4(toLinear(cs.rgb), cs.a);
}
