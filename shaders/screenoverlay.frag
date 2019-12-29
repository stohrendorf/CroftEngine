#include "flat_pipeline_interface.glsl"

uniform sampler2D u_input;
layout(location=0) out vec4 out_color;

void main()
{
    out_color = texture(u_input, fpi.texCoord);
}
