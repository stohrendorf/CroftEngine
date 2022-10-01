#include "csm_interface.glsl"

IN_OUT GeometryPipelineInterface {
    vec3 texCoord;
    vec4 color;
    vec3 vertexPos;
    vec3 vertexPosLight[CSMSplits];
    vec3 vertexPosWorld;
    vec3 vertexNormalWorld;
    vec3 hbaoNormal;

    flat float isQuad;
    flat vec3 quadVerts[4];
    flat vec2 quadUvs[4];

    vec4 reflective;
} gpi;

layout(location=7) uniform int u_lightingMode;