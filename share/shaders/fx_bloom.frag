#include "flat_pipeline_interface.glsl"
#include "fx_input.glsl"

layout(bindless_sampler) uniform sampler2D u_bloom;

void main()
{
    out_color = mix(texture(u_input, fpi.texCoord).rgb, texture(u_bloom, fpi.texCoord).rgb, 0.04);
}
