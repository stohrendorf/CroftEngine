layout(location=11) IN_OUT GeometryPipelineInterface {
    vec2 texCoord;
    vec3 color;
    flat float texIndex;
    vec3 vertexPos;
    vec4 vertexPosLight[3];
    #ifdef WATER
    vec3 vertexPosWorld;
    #endif
    vec3 normal;
    vec3 ssaoNormal;
} gpi;
