const int CSMSplits = 5;

layout(std140, binding=2) uniform CSM {
    mat4 u_lightMVP[CSMSplits];
    vec3 u_csmLightDir;
    float u_csmSplits[CSMSplits];
};
