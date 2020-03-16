#include "flat_pipeline_interface.glsl"

#if BLUR_DIM == 1
#define BLUR_TYPE float
#define BLUR_ACCESSOR x
#elif BLUR_DIM == 2
#define BLUR_TYPE vec2
#define BLUR_ACCESSOR xy
#endif

// factors from http://dev.theomader.com/gaussian-kernel-calculator/
#if BLUR_EXTENT == 1
const float Gauss[3] = { 0.27901, 0.44198, 0.27901 };
#elif BLUR_EXTENT == 2
const float Gauss[5] = { 0.06136, 0.24477, 0.38774, 0.24477, 0.06136 };
#elif BLUR_EXTENT == 3
const float Gauss[7] = { 0.00598, 0.060626, 0.241843, 0.383103, 0.241843, 0.060626, 0.00598 };
#elif BLUR_EXTENT == 4
const float Gauss[9] = { 0.000229, 0.005977, 0.060598, 0.241732, 0.382928, 0.241732, 0.060598, 0.005977, 0.000229 };
#endif

uniform sampler2D u_input;
layout(location=0) out BLUR_TYPE out_tex;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(u_input, 0));
    BLUR_TYPE result = BLUR_TYPE(0.0);
#if BLUR_DIR == 0
    for (int x = -BLUR_EXTENT; x <= BLUR_EXTENT; ++x)
    {
        for (int y = -BLUR_EXTENT; y <= BLUR_EXTENT; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(u_input, fpi.texCoord + offset).BLUR_ACCESSOR * Gauss[x+BLUR_EXTENT] * Gauss[y+BLUR_EXTENT];
        }
    }
        #elif BLUR_DIR == 1
    for (int x = -BLUR_EXTENT; x <= BLUR_EXTENT; ++x)
    {
        vec2 offset = vec2(float(x), 0) * texelSize;
        result += texture(u_input, fpi.texCoord + offset).BLUR_ACCESSOR * Gauss[x+BLUR_EXTENT];
    }
        #elif BLUR_DIR == 2
    for (int y = -BLUR_EXTENT; y <= BLUR_EXTENT; ++y)
    {
        vec2 offset = vec2(0, float(y)) * texelSize;
        result += texture(u_input, fpi.texCoord + offset).BLUR_ACCESSOR * Gauss[y+BLUR_EXTENT];
    }
        #endif
    out_tex = result;
}
