#include "screen_pipeline_interface.glsl"

layout(binding=0) uniform sampler2DArray u_input;
layout(location=0) out vec4 out_color;

void main()
{
    if (spi.texIndex < 0) {
        out_color = spi.color;
        return;
    }

    vec4 color = texture(u_input, vec3(spi.texCoord, spi.texIndex));
    if (color.a < 0.5) {
        discard;
    }
    out_color = color;
}
