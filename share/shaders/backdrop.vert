#define VTX_INPUT_TEXCOORD

#include "vtx_input.glsl"
#include "flat_pipeline_interface.glsl"
#include "camera_interface.glsl"

void main()
{
    vec2 p = (a_position.xy / camera.screenSize.xy) * 2 - 1;
    gl_Position = vec4(p.x, -p.y, 0, 1);
    fpi.texCoord = a_texCoord;
}
