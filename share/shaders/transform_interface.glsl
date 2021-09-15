layout(std140, binding=0) uniform Transform {
    mat4 u_modelMatrix;
};

#ifdef VTX_INPUT_BONE_INDEX
layout(std140) readonly restrict buffer BoneTransform {
    mat4 u_bones[];
};
#endif
