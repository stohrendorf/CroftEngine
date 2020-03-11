layout(std140, binding=2) uniform CSM {
    mat4 u_lightMVP[3];
    float u_csmSplits[3];
    vec3 u_csmLightDir;
};
