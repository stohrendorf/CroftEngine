#include "flat_pipeline_interface.glsl"

layout(bindless_sampler) uniform sampler2D u_input;
layout(location=0) out vec4 out_color;

#ifdef ALPHA_MULTIPLIER
layout(location=15) uniform float u_alphaMultiplier;
#endif

void main()
{
    out_color = texture(u_input, fpi.texCoord);
    #ifdef ALPHA_MULTIPLIER
    out_color.a *= u_alphaMultiplier;
    #endif
}
