#define VTX_INPUT_TEXCOORD

#include "vtx_input.glsl"
#include "screen_pipeline_interface.glsl"

uniform vec2 u_screenSize;

void main()
{
    vec2 p = (a_position.xy / u_screenSize) * 2 - 1;
    gl_Position = vec4(p.x, -p.y, 0, 1);
    spi.color = a_color;
    spi.texCoord = a_texCoord;
    spi.texIndex = a_texIndex;
}
