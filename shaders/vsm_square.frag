#include "flat_pipeline_interface.glsl"

uniform sampler2D u_shadow;
layout(location=0) out vec2 out_tex;

void main()
{
    float d = texture(u_shadow, fpi.texCoord).x;
    out_tex.x = d;
    out_tex.y = d*d;
}
