#include "flat_pipeline_interface.glsl"

layout(bindless_sampler) uniform sampler2D u_input;
layout(bindless_sampler) uniform sampler2D u_ao;
layout(location=0) out vec3 out_color;

void main()
{
    out_color = texture(u_ao, fpi.texCoord).r * texture(u_input, fpi.texCoord).rgb;
}
