#include "flat_pipeline_interface.glsl"

#if BLUR_DIM == 1
#define BLUR_TYPE float
#elif BLUR_DIM == 2
#define BLUR_TYPE vec2
#endif

#ifdef BLUR_GAUSS
// factors from http://dev.theomader.com/gaussian-kernel-calculator/
#if BLUR_EXTENT == 1
const float Gauss[3] = { 0.27901, 0.44198, 0.27901 };
#elif BLUR_EXTENT == 2
const float Gauss[5] = { 0.06136, 0.24477, 0.38774, 0.24477, 0.06136 };
#elif BLUR_EXTENT == 3
const float Gauss[7] = { 0.00598, 0.060626, 0.241843, 0.383103, 0.241843, 0.060626, 0.00598 };
#elif BLUR_EXTENT == 4
const float Gauss[9] = { 0.000229, 0.005977, 0.060598, 0.241732, 0.382928, 0.241732, 0.060598, 0.005977, 0.000229 };
#elif BLUR_EXTENT == 10
// from https://graphics.stanford.edu/~mdfisher/Code/ShadowMap/GaussianBlurX.ps.html
const float Gauss[21] = {
    0.000272337, 0.00089296, 0.002583865, 0.00659813, 0.014869116,
    0.029570767, 0.051898313, 0.080381679, 0.109868729, 0.132526984,
    0.14107424,
    0.132526984, 0.109868729, 0.080381679, 0.051898313, 0.029570767,
    0.014869116, 0.00659813, 0.002583865, 0.00089296, 0.000272337
};
#endif
#define BLUR_MULTIPLIER(i) Gauss[(i)+BLUR_EXTENT]
#else
const float BlurDiv = 1.0 / (2*BLUR_EXTENT + 1);
#define BLUR_MULTIPLIER(i) BlurDiv
#endif

uniform sampler2D u_input;
layout(location=0) out BLUR_TYPE out_tex;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(u_input, 0));
    BLUR_TYPE result = BLUR_TYPE(0.0);
    BLUR_TYPE center = BLUR_TYPE(texture(u_input, fpi.texCoord));
    BLUR_TYPE l, r;
#if BLUR_DIR == 1
    for (int x = -BLUR_EXTENT; x <= BLUR_EXTENT; ++x)
    {
        vec2 offset = vec2(float(x), 0) * texelSize;
        BLUR_TYPE texel = BLUR_TYPE(texture(u_input, fpi.texCoord + offset));
        if(x == -1) l = texel;
        else if(x == 1) r = texel;
        result += texel * BLUR_MULTIPLIER(x);
    }
#elif BLUR_DIR == 2
    for (int y = -BLUR_EXTENT; y <= BLUR_EXTENT; ++y)
    {
        vec2 offset = vec2(0, float(y)) * texelSize;
        BLUR_TYPE texel = BLUR_TYPE(texture(u_input, fpi.texCoord + offset));
        if(y == -1) l = texel;
        else if(y == 1) r = texel;
        result += texel * BLUR_MULTIPLIER(y);
    }
#else
#error "Invalid Blur Dir"
#endif
#ifdef BLUR_CLAMP
    BLUR_TYPE clamped = center.x > result.x ? result : mix(center, result, 0.01);;
    if( center.x > max(l.x, r.x) ) {
        clamped = (l+r)*0.5;
        clamped = l;
    }
    out_tex = clamped;
#else
    out_tex = result;
#endif
}
