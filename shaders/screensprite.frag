#ifndef SCREENSPRITE_TEXTURE
#define VTX_INPUT_COLOR_QUAD
#endif

#include "screen_pipeline_interface.glsl"

layout(binding=0) uniform sampler2DArray u_input;
layout(location=0) out vec4 out_color;

void main()
{
    #ifdef SCREENSPRITE_TEXTURE
    out_color = texture(u_input, vec3(spi.texCoord, spi.texIndex));
    #else
    vec4 top = mix(spi.topLeft, spi.topRight, spi.texCoord.x);
    vec4 bottom = mix(spi.bottomLeft, spi.bottomRight, spi.texCoord.x);
    out_color = mix(top, bottom, spi.texCoord.y);
    #endif
}
