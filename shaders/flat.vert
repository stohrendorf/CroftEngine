#define VTX_INPUT_TEXCOORD

#include "vtx_input.glsl"
#include "flat_pipeline_interface.glsl"

uniform mat4 u_projection;

void main()
{
    gl_Position = u_projection * vec4(a_position, 1);
    fpi.texCoord = a_texCoord;
}
