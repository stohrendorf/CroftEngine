layout(std140, binding=0) uniform Transform {
    mat4 m;
} modelTransform;

#ifdef SKELETAL
layout(std140) readonly restrict buffer BoneTransform {
    mat4 m[];
} boneTransform;
#endif
