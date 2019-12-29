#define VTX_INPUT_TEXCOORD

#include "vtx_input.glsl"
#include "flat_pipeline_interface.glsl"

void main()
{
    vec2 p = a_texCoord.xy * 2 - 1;
    #ifdef INVERT_Y
    p.y *= -1;
    #endif
    gl_Position = vec4(p.x, p.y, 0, 1);
    fpi.texCoord = a_texCoord;
}
