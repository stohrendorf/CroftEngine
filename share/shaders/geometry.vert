#define VTX_INPUT_NORMAL
#define VTX_INPUT_TEXCOORD
#define VTX_INPUT_TEXCOORD_QUAD
#ifdef SKELETAL
#define VTX_INPUT_BONE_INDEX
#endif

#include "vtx_input.glsl"
#include "transform_interface.glsl"
#include "geometry_pipeline_interface.glsl"
#include "camera_interface.glsl"

void main()
{
    #ifdef SKELETAL
    mat4 mm = modelTransform.m * boneTransform.m[int(a_boneIndex)];
    #else
    mat4 mm = modelTransform.m;
    #endif
    mat4 mv = camera.view * mm;

    #if SPRITEMODE == 1
    mv[0].xyz = vec3(1, 0, 0);
    mv[2].xyz = vec3(0, 0, 1);
    #elif SPRITEMODE == 2
    mv[0].xyz = vec3(1, 0, 0);
    mv[1].xyz = vec3(0, 1, 0);
    mv[2].xyz = vec3(0, 0, 1);
    #endif

    vec4 tmp = mv * vec4(a_position, 1);
    gpi.vertexPos = tmp.xyz;
    gpi.vertexPosWorld = vec3(mm * vec4(a_position, 1.0));
    gl_Position = camera.projection * tmp;
    gpi.texCoord = a_texCoord;
    gpi.texIndex = a_texIndex;
    gpi.color = a_color;

    gpi.vertexNormalWorld = normalize(mat3(mm) * a_normal);
    gpi.hbaoNormal = normalize(mat3(mv) * a_normal);
    float dist = 16 * clamp(1.0 - dot(normalize(csm.lightDir), gpi.vertexNormalWorld), 0.0, 1.0);
    vec4 pos = vec4(a_position + dist * gpi.vertexNormalWorld, 1.0);
    for (int i=0; i<CSMSplits; ++i)
    {
        #ifdef SKELETAL
        mat4 lmvp = csm.lightMVP[i] * boneTransform.m[int(a_boneIndex)];
        #else
        mat4 lmvp = csm.lightMVP[i];
        #endif
        vec4 tmp = lmvp * pos;
        gpi.vertexPosLight[i] = (tmp.xyz / tmp.w) * 0.5 + 0.5;
    }

    gpi.isQuad = a_isQuad;
    if (a_isQuad != 0)
    {
        mat4 mvp = camera.projection * mv;

        vec4 tmp = mvp * vec4(a_quadVert1, 1);
        gpi.quadVerts[0] = vec3(tmp.xy / tmp.w, tmp.w);
        tmp = mvp * vec4(a_quadVert2, 1);
        gpi.quadVerts[1] = vec3(tmp.xy / tmp.w, tmp.w);
        tmp = mvp * vec4(a_quadVert3, 1);
        gpi.quadVerts[2] = vec3(tmp.xy / tmp.w, tmp.w);
        tmp = mvp * vec4(a_quadVert4, 1);
        gpi.quadVerts[3] = vec3(tmp.xy / tmp.w, tmp.w);

        gpi.quadUvs[0] = a_quadUv1;
        gpi.quadUvs[1] = a_quadUv2;
        gpi.quadUvs[2] = a_quadUv3;
        gpi.quadUvs[3] = a_quadUv4;
    }
}
