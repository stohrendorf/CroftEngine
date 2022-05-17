#include "geometry_pipeline_interface.glsl"

layout(location=0) out vec4 out_color;
layout(location=1) out vec3 out_normal;
layout(location=2) out vec3 out_position;

void main()
{
    out_normal = gpi.hbaoNormal;
    out_position = gpi.vertexPos;
    float y = abs(mod(out_position.y, 16.0)-8.0) / 8.0;
    out_color = vec4(0.2, 0.2, 0.8, 1) * 0.8 * y;
}
