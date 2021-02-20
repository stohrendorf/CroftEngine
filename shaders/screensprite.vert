#define VTX_INPUT_TEXCOORD

#ifndef SCREENSPRITE_TEXTURE
#define VTX_INPUT_COLOR_QUAD
#endif

#include "vtx_input.glsl"
#include "screen_pipeline_interface.glsl"

uniform vec2 u_screenSize;

void main()
{
    vec2 p = (a_position.xy / u_screenSize) * 2 - 1;
    gl_Position = vec4(p.x, -p.y, 0, 1);
    spi.texCoord = a_texCoord;
    #ifdef SCREENSPRITE_TEXTURE
    spi.texIndex = a_texIndex;
    #else
    spi.topLeft = a_colorTopLeft;
    spi.topRight = a_colorTopRight;
    spi.bottomLeft = a_colorBottomLeft;
    spi.bottomRight = a_colorBottomRight;
    #endif
}
