#include "flat_pipeline_interface.glsl"

#if BLUR_DIM == 1
#define BLUR_TYPE float
#elif BLUR_DIM == 2
#define BLUR_TYPE vec2
#elif BLUR_DIM == 3
#define BLUR_TYPE vec3
#endif

layout(bindless_sampler) uniform sampler2D u_input;
layout(location=0) uniform int u_blurDir;
layout(location=0) out BLUR_TYPE out_tex;

void main()
{
    const vec2 texelSize = 1.0 / vec2(textureSize(u_input, 0));

    vec2 direction = u_blurDir == 1 ? vec2(1, 0) * texelSize : vec2(0, 1) * texelSize;
    vec2 off1 = vec2(1 + 1.0/3.0) * direction;
    out_tex = (
    BLUR_TYPE(texture(u_input, fpi.texCoord)) * 0.29411764705882354
    + BLUR_TYPE(texture(u_input, fpi.texCoord + off1)) * 0.35294117647058826
    + BLUR_TYPE(texture(u_input, fpi.texCoord - off1)) * 0.35294117647058826
    );
}
