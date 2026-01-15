#include "vtx_input.glsl"
#include "transform_interface.glsl"

uniform mat4 u_mvp;

void main()
{
    #ifdef SKELETAL
    gl_Position = mix(
    u_mvp * boneTransform.m[int(a_boneIndex)] * vec4(a_position, 1),
    u_mvp * nextBoneTransform.m[int(a_boneIndex)] * vec4(a_position, 1),
    u_interTickFactor
    );
    #else
    gl_Position = u_mvp * vec4(a_position, 1);
    #endif
}
