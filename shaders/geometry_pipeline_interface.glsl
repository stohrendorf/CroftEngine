layout(location=11) IN_OUT GeometryPipelineInterface {
    vec2 texCoord;
    vec4 color;
    flat float texIndex;
    vec3 vertexPos;
    vec3 vertexPosLight1;
    vec3 vertexPosLight2;
    vec3 vertexPosLight3;
    vec3 vertexPosLight4;
    vec3 vertexPosLight5;
    vec3 vertexPosWorld;
    vec3 normal;
    vec3 ssaoNormal;

    flat float isQuad;

    flat vec3 quadV1;
    flat vec3 quadV2;
    flat vec3 quadV3;
    flat vec3 quadV4;

    flat vec2 quadUv1;
    flat vec2 quadUv2;
    flat vec2 quadUv3;
    flat vec2 quadUv4;
} gpi;
