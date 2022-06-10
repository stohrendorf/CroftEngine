#include "geometry_pipeline_interface.glsl"
#include "camera_interface.glsl"

layout(bindless_sampler) uniform sampler2D u_texture;
uniform vec3 u_color;

layout(location=0) out vec4 out_color;
layout(location=1) out vec3 out_normal;
layout(location=2) out vec3 out_position;
layout(location=3) out vec4 out_reflective;

void main()
{
    out_normal = gpi.hbaoNormal;
    out_position = gpi.vertexPos;
    out_reflective = vec4(0);

    float a = texture(u_texture, gpi.texCoord.xy).x;
    if (a == 0) {
        discard;
    }
    out_color = vec4(a * u_color, a);
}
