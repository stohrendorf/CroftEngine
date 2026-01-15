#include "time_uniform.glsl"
#include "camera_interface.glsl"

float texel_shade(in float depth)
{
    float s = clamp(1.0 - depth * InvFarPlane, 0.0, 1.0);
    return s * sqrt(s);
}

// https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_sRGB.txt
float toSrgb(float cl)
{
    float s1 = 12.92 * cl;
    float s2 = 1.055 * pow(cl, 0.41666) - 0.055;
    return mix(s2, s1, step(cl, 0.0031308));
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
    float l1 = cs / 12.92;
    float l2 = pow((cs + 0.055) / 1.055, 2.4);
    return mix(l2, l1, step(cs, 0.04045));
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

float luminanceLinear(in vec3 color)
{
    return dot(color, vec3(0.212656, 0.715158, 0.072186));
}

float luminanceLinear(in vec4 color)
{
    return dot(vec3(color), vec3(0.212656, 0.715158, 0.072186));
}

float luminanceRgb(in vec3 color)
{
    return luminanceLinear(toLinear(color));
}

float luminanceRgb(in vec4 color)
{
    return luminanceLinear(toLinear(color));
}
