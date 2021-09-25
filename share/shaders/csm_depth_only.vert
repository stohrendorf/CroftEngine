#ifdef SKELETAL
#define VTX_INPUT_BONE_INDEX
#endif

#include "vtx_input.glsl"
#include "transform_interface.glsl"

uniform mat4 u_mvp;

void main()
{
    #ifdef SKELETAL
    gl_Position = u_mvp * boneTransform.m[int(a_boneIndex)] * vec4(a_position, 1);
    #else
    gl_Position = u_mvp * vec4(a_position, 1);
    #endif
}
