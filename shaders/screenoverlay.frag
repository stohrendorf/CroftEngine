uniform sampler2D u_texture;

#include "screenoverlay_pipeline_interface.glsl"

layout(location=0) out vec4 out_color;

void main()
{
    out_color = texture(u_texture, sopi.texCoord);
}
