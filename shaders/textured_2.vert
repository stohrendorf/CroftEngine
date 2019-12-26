#define VTX_INPUT_NORMAL
#define VTX_INPUT_COLOR
#define VTX_INPUT_TEXCOORD

#include "vtx_input.glsl"
#include "transform_interface.glsl"
#include "csm_interface.glsl"
#include "geometry_pipeline_interface.glsl"

void main()
{
    vec4 tmp = u_modelViewMatrix * vec4(a_position, 1);
    #ifdef WATER
    gpi.vertexPosWorld = vec3(u_modelMatrix * vec4(a_position, 1));
    #endif
    gl_Position = u_camProjection * tmp;
    gpi.texCoord = a_texCoord;
    gpi.color = a_color;

    gpi.normal = normalize(mat3(u_modelMatrix) * a_normal);
    gpi.ssaoNormal = normalize(mat3(u_modelViewMatrix) * a_normal);
    gpi.vertexPos = tmp.xyz;
    for (int i=0; i<3; ++i)
    gpi.vertexPosLight[i] = u_lightMVP[i] * vec4(a_position, 1);
}
