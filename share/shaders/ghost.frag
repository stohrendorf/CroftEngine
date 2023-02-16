#include "geometry_pipeline_interface.glsl"

layout(location=0) out vec4 out_color;
layout(location=1) out vec3 out_normal;
layout(location=2) out vec3 out_position;

layout(location=0) uniform vec3 u_color;

void main()
{
    out_normal = gpi.hbaoNormal;
    out_position = gpi.vertexPos;
    out_color = clamp(vec4(u_color, 1.0) + vec4(vec3(max(0.0, pow(-gpi.hbaoNormal.z, 1.45))), 0), vec4(0.0), vec4(1.0));
}
