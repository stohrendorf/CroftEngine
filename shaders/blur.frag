#include "flat_pipeline_interface.glsl"

#if BLUR_DIM == 1
#define BLUR_TYPE float
#define BLUR_ACCESSOR x
#elif BLUR_DIM == 2
#define BLUR_TYPE vec2
#define BLUR_ACCESSOR xy
#endif

uniform sampler2D u_input;
layout(location=0) out BLUR_TYPE out_tex;

const int BlurExtent = BLUR_EXTENT;
const float BlurSize = BlurExtent*2 + 1;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(u_input, 0));
    BLUR_TYPE result = BLUR_TYPE(0.0);
#if BLUR_DIR == 0
    for (int x = -BlurExtent; x <= BlurExtent; ++x)
    {
        for (int y = -BlurExtent; y <= BlurExtent; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(u_input, fpi.texCoord + offset).BLUR_ACCESSOR;
        }
    }
    out_tex = result / (BlurSize * BlurSize);
    #elif BLUR_DIR == 1
    for (int x = -BlurExtent; x <= BlurExtent; ++x)
    {
        vec2 offset = vec2(float(x), 0) * texelSize;
        result += texture(u_input, fpi.texCoord + offset).BLUR_ACCESSOR;
    }
    out_tex = result / BlurSize;
    #elif BLUR_DIR == 2
    for (int y = -BlurExtent; y <= BlurExtent; ++y)
    {
        vec2 offset = vec2(0, float(y)) * texelSize;
        result += texture(u_input, fpi.texCoord + offset).BLUR_ACCESSOR;
    }
    out_tex = result / BlurSize;
    #endif
}
