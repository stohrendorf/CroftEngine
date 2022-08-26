#include "geometry_pipeline_interface.glsl"

layout(location=0) out vec4 out_color;
layout(location=1) out vec3 out_normal;
layout(location=2) out vec3 out_position;

void main()
{
    out_normal = gpi.hbaoNormal;
    out_position = gpi.vertexPos;
    vec3 color = vec3(0.2, 0.2, 0.8) * 0.8;
    out_color = clamp(vec4(color, 1.0) + vec4(vec3(pow(-gpi.hbaoNormal.z, 1.45)), 0), vec4(0.0), vec4(1.0));
}
