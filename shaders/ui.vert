#define VTX_INPUT_TEXCOORD
#define VTX_INPUT_COLOR_QUAD

#include "vtx_input.glsl"
#include "ui_pipeline_interface.glsl"

uniform vec2 u_screenSize;

void main()
{
    vec2 p = (a_position.xy / u_screenSize) * 2 - 1;
    gl_Position = vec4(p.x, -p.y, 0, 1);
    upi.texCoord = a_texCoord;
    upi.texIndex = a_texIndex;
    upi.topLeft = a_colorTopLeft;
    upi.topRight = a_colorTopRight;
    upi.bottomLeft = a_colorBottomLeft;
    upi.bottomRight = a_colorBottomRight;
}
