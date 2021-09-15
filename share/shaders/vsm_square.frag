#include "flat_pipeline_interface.glsl"

layout(bindless_sampler) uniform sampler2D u_shadow;
layout(location=0) out vec2 out_tex;

void main()
{
    float d = texture(u_shadow, fpi.texCoord).x;
    out_tex = vec2(d, d*d);
}
