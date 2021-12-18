#include "vtx_input.glsl"
#include "transform_interface.glsl"
#include "geometry_pipeline_interface.glsl"
#include "camera_interface.glsl"

void main()
{
    mat4 mm = modelTransform.m * boneTransform.m[int(a_boneIndex)];
    mat4 mv = camera.view * mm;

    vec4 mvPos = mv * vec4(a_position, 1.0);
    gpi.vertexPos = mvPos.xyz;
    gpi.vertexPosWorld = vec3(mm * vec4(a_position, 1.0));
    gl_Position = camera.projection * mvPos;
    gpi.texCoord = a_texCoord;
    gpi.color = a_color;

    gpi.vertexNormalWorld = normalize(mat3(mm) * a_normal);
    gpi.hbaoNormal = normalize(mat3(mv) * a_normal);
}
