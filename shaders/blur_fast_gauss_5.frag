#include "flat_pipeline_interface.glsl"

#if BLUR_DIM == 1
#define BLUR_TYPE float
#elif BLUR_DIM == 2
#define BLUR_TYPE vec2
#endif

uniform sampler2D u_input;
layout(location=0) out BLUR_TYPE out_tex;

void main()
{
    const vec2 texelSize = 1.0 / vec2(textureSize(u_input, 0));

    #if BLUR_DIR == 1
    vec2 direction = vec2(1, 0) * texelSize;
    #elif BLUR_DIR == 2
    vec2 direction = vec2(0, 1) * texelSize;
    #else
    #error "Invalid Blur Dir"
    #endif
    vec2 off1 = vec2(1 + 1.0/3.0) * direction;
    out_tex = (
    BLUR_TYPE(texture(u_input, fpi.texCoord)) * 0.29411764705882354
    + BLUR_TYPE(texture(u_input, fpi.texCoord + off1)) * 0.35294117647058826
    + BLUR_TYPE(texture(u_input, fpi.texCoord - off1)) * 0.35294117647058826
    );
}
