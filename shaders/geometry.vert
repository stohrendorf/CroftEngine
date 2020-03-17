#define VTX_INPUT_NORMAL
#define VTX_INPUT_TEXCOORD

#include "vtx_input.glsl"
#include "transform_interface.glsl"
#include "csm_interface.glsl"
#include "geometry_pipeline_interface.glsl"
#include "camera_interface.glsl"

uniform int u_spritePole = -1;

void main()
{
    mat4 mv = u_view * u_modelMatrix;

    if (u_spritePole >= 0) {
        if (u_spritePole != 0) {
            mv[0].xyz = vec3(1, 0, 0);
        }
        if (u_spritePole != 1) {
            mv[1].xyz = vec3(0, 1, 0);
        }
        if (u_spritePole != 2) {
            mv[2].xyz = vec3(0, 0, 1);
        }
    }

    vec4 tmp = mv * vec4(a_position, 1);
    #ifdef WATER
    gpi.vertexPosWorld = vec3(u_modelMatrix * vec4(a_position, 1));
    #endif
    gl_Position = u_projection * tmp;
    gpi.texCoord = a_texCoord;
    gpi.texIndex = a_texIndex;
    gpi.color = a_color;

    gpi.normal = normalize(mat3(u_modelMatrix) * a_normal);
    gpi.ssaoNormal = normalize(mat3(mv) * a_normal);
    gpi.vertexPos = tmp.xyz;
    for (int i=0; i<3; ++i) {
        gpi.vertexPosLight[i] = u_lightMVP[i] * vec4(a_position, 1);
    }
}
