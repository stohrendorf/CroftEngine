layout(std140, binding=2) uniform CSM {
    mat4 u_lightMVP1;
    mat4 u_lightMVP2;
    mat4 u_lightMVP3;
    mat4 u_lightMVP4;
    mat4 u_lightMVP5;
    vec3 u_csmLightDir;
    float u_csmSplits1;
    float u_csmSplits2;
    float u_csmSplits3;
    float u_csmSplits4;
    float u_csmSplits5;
};
