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

// Nearest emulated sample given floating point position and texel offset.
// Also zero's off screen.
vec3 Fetch(in ivec2 off, in vec2 res)
{
    return toLinear(texelFetch(u_input, ivec2(fpi.texCoord*res) + off, 0).rgb);
}

// Distance in emulated pixels to nearest texel.
vec2 Dist(in vec2 res)
{
    vec2 st = fpi.texCoord*res;
    return -(st - floor(st) - vec2(0.5));
}

// 1D Gaussian.
float Gaus(in float x, in float scale)
{
    return exp2(scale*x*x);
}

// 3-tap Gaussian filter along horz line.
vec3 Horz3(in int off, in vec2 res)
{
    vec3 b=Fetch(ivec2(-1, off), res);
    vec3 c=Fetch(ivec2(0, off), res);
    vec3 d=Fetch(ivec2(1, off), res);
    float dst=Dist(res).x;
    // Convert distance to weight.
    float wb=Gaus(dst-1.0, hardPix);
    float wc=Gaus(dst+0.0, hardPix);
    float wd=Gaus(dst+1.0, hardPix);
    // Return filtered sample.
    return (b*wb+c*wc+d*wd)/(wb+wc+wd);
}

// 5-tap Gaussian filter along horz line.
vec3 Horz5(int off, in vec2 res)
{
    vec3 a=Fetch(ivec2(-2, off), res);
    vec3 b=Fetch(ivec2(-1, off), res);
    vec3 c=Fetch(ivec2(0, off), res);
    vec3 d=Fetch(ivec2(1, off), res);
    vec3 e=Fetch(ivec2(2, off), res);
    float dst=Dist(res).x;
    // Convert distance to weight.
    float wa=Gaus(dst-2.0, hardPix);
    float wb=Gaus(dst-1.0, hardPix);
    float wc=Gaus(dst+0.0, hardPix);
    float wd=Gaus(dst+1.0, hardPix);
    float we=Gaus(dst+2.0, hardPix);
    // Return filtered sample.
    return (a*wa+b*wb+c*wc+d*wd+e*we)/(wa+wb+wc+wd+we);
}

// Return scanline weight.
float Scan(in float off, in vec2 res)
{
    return Gaus(Dist(res/4).y + off, hardScan);
}

// Allow nearest three lines to effect pixel.
vec3 Tri(in vec2 res)
{
    vec3 a=Horz3(-1, res);
    vec3 b=Horz5(0, res);
    vec3 c=Horz3(1, res);
    float wa=Scan(-1.0, res);
    float wb=Scan(0.0, res);
    float wc=Scan(1.0, res);
    return a*wa+b*wb+c*wc;
}

// Shadow mask.
vec3 Mask(in vec2 res)
{
    vec2 pos = fpi.texCoord * res;
    pos.x += pos.y*3.0;
    vec3 mask = vec3(maskDark, maskDark, maskDark);
    pos.x = fract(pos.x/6.0);
    if (pos.x < 0.333) {
        mask.r = maskLight;
    }
    else if (pos.x < 0.666) {
        mask.g = maskLight;
    }
    else {
        mask.b = maskLight;
    }
    return mask;
}

void main()
{
    vec2 res = textureSize(u_input, 0).xy;
    out_color = toSrgb(Tri(res) * Mask(res));
}
