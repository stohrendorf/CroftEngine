#include "flat_pipeline_interface.glsl"

layout(bindless_sampler) uniform sampler2D u_input;
layout(location=0) out vec3 out_color;

void main()
{
    const float VelviaAmount = 0.03;
    const vec2 VelviaFac = vec2(2*VelviaAmount + 1.0, -VelviaAmount);
    vec3 texel = texture(u_input, fpi.texCoord).rgb;
    vec3 velviaColor = vec3(dot(texel, VelviaFac.xyy), dot(texel, VelviaFac.yxy), dot(texel, VelviaFac.yyx));
    out_color = vec3(1.0) - clamp((-velviaColor*1.01 + vec3(1.0))*1.01, vec3(0.0), vec3(1.0));
}
