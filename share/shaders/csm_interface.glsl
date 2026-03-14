#include "bindings.glsl"

const int CSMSplits = 4;

layout(std140, binding=BINDING_UNIFORM_CSM) uniform CSM {
    mat4 lightMVP[CSMSplits];
    vec3 lightDir;
} csm;
