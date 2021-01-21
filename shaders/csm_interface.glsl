layout(std140, binding=2) uniform CSM {
    mat4 u_lightMVP1;
    mat4 u_lightMVP2;
    mat4 u_lightMVP3;
    vec3 u_csmLightDir;
    float u_csmSplits1;
    float u_csmSplits2;
    float u_csmSplits3;
};
