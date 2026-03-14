#include "vtx_input.glsl"
#include "transform_interface.glsl"
#include "camera_interface.glsl"

#include "geometry_pipeline_interface.glsl"

void main()
{
    gpi.texCoord = a_texCoord;
    gpi.color = a_color;

    mat4 mvp = camera.viewProjection * modelTransform.m;

    #ifdef SKELETAL
    vec4 vtx = mix(
    mvp * boneTransform.m[int(a_boneIndex)] * vec4(a_position, 1),
    mvp * nextBoneTransform.m[int(a_boneIndex)] * vec4(a_position, 1),
    u_interTickFactor
    );
    #else
    vec4 vtx = mvp * vec4(a_position, 1);
    #endif
    vtx.z = (vtx.z / vtx.w + 1/512.0) * vtx.w;// depth offset
    gl_Position = vtx;
}
