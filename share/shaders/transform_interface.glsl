#include "bindings.glsl"

layout(std140, binding=BINDING_UNIFORM_TRANSFORM) uniform Transform {
    mat4 m;
} modelTransform;

#ifdef SKELETAL
uniform float u_interTickFactor;
layout(std140, binding=BINDING_BUFFER_BONE_TRANSFORM) readonly restrict buffer BoneTransform {
    mat4 m[];
} boneTransform;
layout(std140, binding=BINDING_BUFFER_NEXT_BONE_TRANSFORM) readonly restrict buffer NextBoneTransform {
    mat4 m[];
} nextBoneTransform;
#endif
