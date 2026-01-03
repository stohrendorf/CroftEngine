// based on https://www.shadertoy.com/view/XsjSzR

#include "flat_pipeline_interface.glsl"
#include "fx_input.glsl"
#include "util.glsl"

// Hardness of scanline.
//  -8.0 = soft
// -16.0 = medium
const float hardScan=-8.0;

// Hardness of pixels in scanline.
// -2.0 = soft
// -4.0 = hard
const float hardPix=-3.0;

// Amount of shadow mask.
const float maskDark=0.5;
const float maskLight=1.5;

//------------------------------------------------------------------------

// Allow nearest three lines to effect pixel.
vec3 Tri(in vec2 res)
{
    vec2 st = fpi.texCoord * res;
    ivec2 center = ivec2(st);
    vec2 dst = -(st - floor(st) - 0.5);

    // Horizontal weights
    float w0 = exp2(hardPix * (dst.x - 2.0) * (dst.x - 2.0));
    float w1 = exp2(hardPix * (dst.x - 1.0) * (dst.x - 1.0));
    float w2 = exp2(hardPix * (dst.x + 0.0) * (dst.x + 0.0));
    float w3 = exp2(hardPix * (dst.x + 1.0) * (dst.x + 1.0));
    float w4 = exp2(hardPix * (dst.x + 2.0) * (dst.x + 2.0));

    float sum3 = 1.0 / (w1 + w2 + w3);
    float sum5 = 1.0 / (w0 + w1 + w2 + w3 + w4);

    // Row -1
    vec3 fetch_m1_m1 = toLinear(texelFetch(u_input, center + ivec2(-1, -1), 0).rgb);
    vec3 fetch_0_m1  = toLinear(texelFetch(u_input, center + ivec2(0, -1), 0).rgb);
    vec3 fetch_1_m1  = toLinear(texelFetch(u_input, center + ivec2(1, -1), 0).rgb);
    vec3 a = (fetch_m1_m1 * w1 + fetch_0_m1 * w2 + fetch_1_m1 * w3) * sum3;

    // Row 0
    vec3 fetch_m2_0  = toLinear(texelFetch(u_input, center + ivec2(-2, 0), 0).rgb);
    vec3 fetch_m1_0  = toLinear(texelFetch(u_input, center + ivec2(-1, 0), 0).rgb);
    vec3 fetch_0_0   = toLinear(texelFetch(u_input, center + ivec2(0, 0), 0).rgb);
    vec3 fetch_1_0   = toLinear(texelFetch(u_input, center + ivec2(1, 0), 0).rgb);
    vec3 fetch_2_0   = toLinear(texelFetch(u_input, center + ivec2(2, 0), 0).rgb);
    vec3 b = (fetch_m2_0 * w0 + fetch_m1_0 * w1 + fetch_0_0 * w2 + fetch_1_0 * w3 + fetch_2_0 * w4) * sum5;

    // Row 1
    vec3 fetch_m1_1  = toLinear(texelFetch(u_input, center + ivec2(-1, 1), 0).rgb);
    vec3 fetch_0_1   = toLinear(texelFetch(u_input, center + ivec2(0, 1), 0).rgb);
    vec3 fetch_1_1   = toLinear(texelFetch(u_input, center + ivec2(1, 1), 0).rgb);
    vec3 c = (fetch_m1_1 * w1 + fetch_0_1 * w2 + fetch_1_1 * w3) * sum3;

    // Scanline weights
    vec2 st4 = st * 0.25;
    float dst4y = -(st4.y - floor(st4.y) - 0.5);
    float wa = exp2(hardScan * (dst4y - 1.0) * (dst4y - 1.0));
    float wb = exp2(hardScan * (dst4y + 0.0) * (dst4y + 0.0));
    float wc = exp2(hardScan * (dst4y + 1.0) * (dst4y + 1.0));

    return a*wa + b*wb + c*wc;
}

// Shadow mask.
vec3 Mask(in vec2 res)
{
    vec2 pos = fpi.texCoord * res;
    pos.x += pos.y * 3.0;
    float x = fract(pos.x / 6.0);
    vec3 mask = vec3(maskDark);
    if (x < 0.333) mask.r = maskLight;
    else if (x < 0.666) mask.g = maskLight;
    else mask.b = maskLight;
    return mask;
}

void main()
{
    vec2 res = textureSize(u_input, 0).xy;
    out_color = toSrgb(Tri(res) * Mask(res));
}
