#include "flat_pipeline_interface.glsl"
#include "fx_input.glsl"

layout(bindless_sampler) uniform sampler2D u_ao;

void main()
{
    out_color = texture(u_ao, fpi.texCoord).r * texture(u_input, fpi.texCoord).rgb;
}
