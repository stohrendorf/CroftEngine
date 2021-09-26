#include "ui_pipeline_interface.glsl"

layout(bindless_sampler) uniform sampler2DArray u_input;
layout(location=0) out vec4 out_color;

void main()
{
    if (upi.texCoord.z >= 0) {
        out_color = texture(u_input, upi.texCoord);
    }
    else {
        vec4 top = mix(upi.topLeft, upi.topRight, upi.texCoord.x);
        vec4 bottom = mix(upi.bottomLeft, upi.bottomRight, upi.texCoord.x);
        out_color = mix(top, bottom, upi.texCoord.y);
    }
}
