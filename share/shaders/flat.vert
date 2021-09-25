#define VTX_INPUT_TEXCOORD

#include "vtx_input.glsl"
#include "flat_pipeline_interface.glsl"
#include "camera_interface.glsl"

#ifdef ASPECT_RATIO
layout(bindless_sampler) uniform sampler2D u_input;
#endif

void main()
{
    vec2 p = a_texCoord.xy * 2 - 1;
    #ifdef INVERT_Y
    p.y *= -1;
    #endif

    #ifdef ASPECT_RATIO
    vec2 inSize = vec2(textureSize(u_input, 0));
    float inAspect = inSize.x / inSize.y;
    if (inAspect > camera.aspectRatio) {
        p.y *= camera.aspectRatio / inAspect;
    }
    else {
        p.x *= inAspect / camera.aspectRatio;
    }
        #endif

    gl_Position = vec4(p.x, p.y, 0, 1);
    fpi.texCoord = a_texCoord;
}
