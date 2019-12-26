#define VTX_INPUT_NORMAL
#define VTX_INPUT_TEXCOORD

#include "vtx_input.glsl"
#include "screenoverlay_pipeline_interface.glsl"

uniform mat4 u_projection;

void main()
{
    gl_Position = u_projection * vec4(a_position, 1);
    sopi.texCoord = a_texCoord;
}
