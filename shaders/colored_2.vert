#define VTX_INPUT_NORMAL
#define VTX_INPUT_COLOR

#include "vtx_input.glsl"
#include "csm_interface.glsl"
#include "transform_interface.glsl"
#include "geometry_pipeline_interface.glsl"
#include "camera_interface.glsl"

void main()
{
    mat4 mv = u_view * u_modelMatrix;
    vec4 tmp = mv * vec4(a_position, 1);
    gl_Position = u_projection * tmp;
    gpi.color = a_color;

    gpi.normal = normalize(mat3(u_modelMatrix) * a_normal);
    gpi.ssaoNormal = normalize(mat3(mv) * a_normal);
    gpi.vertexPos = tmp.xyz;
    for (int i=0; i<3; ++i) {
        gpi.vertexPosLight[i] = u_lightMVP[i] * vec4(a_position, 1);
    }
}
