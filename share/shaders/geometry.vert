#include "vtx_input.glsl"
#include "transform_interface.glsl"
#include "geometry_pipeline_interface.glsl"
#include "camera_interface.glsl"

#include "util.glsl"

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

    vec4 mvPos = mv * vec4(a_position, 1.0);
    gpi.vertexPos = mvPos.xyz;
    gpi.vertexPosWorld = vec3(mm * vec4(a_position, 1.0));
    gl_Position = camera.projection * mvPos;
    gpi.texCoord = a_texCoord;
    gpi.color = gpi.texCoord.z >= 0 ? a_color : toLinear(a_color);

    gpi.vertexNormalWorld = normalize(mat3(mm) * a_normal);
    gpi.hbaoNormal = normalize(mat3(mv) * a_normal);
    vec4 pos = vec4(a_position, 1.0);
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

    {
        gpi.isQuad = a_isQuad;

        mat4 mvp = camera.projection * mv;

        vec4 tmp = mvp * vec4(a_quadVert1, 1);
        gpi.quadVerts[0] = vec3(tmp.xy / tmp.w, tmp.w);
        tmp = mvp * vec4(a_quadVert2, 1);
        gpi.quadVerts[1] = vec3(tmp.xy / tmp.w, tmp.w);
        tmp = mvp * vec4(a_quadVert3, 1);
        gpi.quadVerts[2] = vec3(tmp.xy / tmp.w, tmp.w);
        tmp = mvp * vec4(a_quadVert4, 1);
        gpi.quadVerts[3] = vec3(tmp.xy / tmp.w, tmp.w);

        gpi.quadUvs[0] = a_quadUv12.xy;
        gpi.quadUvs[1] = a_quadUv12.zw;
        gpi.quadUvs[2] = a_quadUv34.xy;
        gpi.quadUvs[3] = a_quadUv34.zw;
    }
}
