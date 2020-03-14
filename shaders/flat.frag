#include "flat_pipeline_interface.glsl"

layout(binding=0) uniform sampler2D u_input;
layout(location=0) out vec4 out_color;

void main()
{
    out_color = texture(u_input, fpi.texCoord);
}
