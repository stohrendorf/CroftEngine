#define VTX_INPUT_NORMAL
#define VTX_INPUT_TEXCOORD
#ifdef SKELETAL
#define VTX_INPUT_BONE_INDEX
#endif

#include "vtx_input.glsl"
#include "transform_interface.glsl"
#include "csm_interface.glsl"
#include "geometry_pipeline_interface.glsl"
#include "camera_interface.glsl"

uniform int u_spritePole = -1;

void main()
{
    #ifdef SKELETAL
    mat4 mm = u_modelMatrix * u_bones[int(a_boneIndex)];
    #else
    mat4 mm = u_modelMatrix;
    #endif
    mat4 mv = u_view * mm;

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
    gpi.vertexPosWorld = vec3(mm * vec4(a_position, 1));
    #endif
    gl_Position = u_projection * tmp;
    gpi.texCoord = a_texCoord;
    gpi.texIndex = a_texIndex;
    gpi.color = a_color;

    gpi.normal = normalize(mat3(mm) * a_normal);
    gpi.ssaoNormal = normalize(mat3(mv) * a_normal);
    gpi.vertexPos = tmp.xyz;
    float dist = 16 * clamp(1.0 - dot(normalize(u_csmLightDir), gpi.normal), 0.0, 1.0);
    vec4 pos = vec4(a_position + dist * gpi.normal, 1);
    {
        #ifdef SKELETAL
        mat4 lmvp = u_lightMVP1 * u_bones[int(a_boneIndex)];
        #else
        mat4 lmvp = u_lightMVP1;
        #endif
        vec4 tmp = lmvp * pos;
        gpi.vertexPosLight1 = tmp.xyz / tmp.w * 0.5 + 0.5;
    }
    {
        #ifdef SKELETAL
        mat4 lmvp = u_lightMVP2 * u_bones[int(a_boneIndex)];
        #else
        mat4 lmvp = u_lightMVP2;
        #endif
        vec4 tmp = lmvp * pos;
        gpi.vertexPosLight2 = tmp.xyz / tmp.w * 0.5 + 0.5;
    }
    {
        #ifdef SKELETAL
        mat4 lmvp = u_lightMVP3 * u_bones[int(a_boneIndex)];
        #else
        mat4 lmvp = u_lightMVP3;
        #endif
        vec4 tmp = lmvp * pos;
        gpi.vertexPosLight3 = tmp.xyz / tmp.w * 0.5 + 0.5;
    }
}
