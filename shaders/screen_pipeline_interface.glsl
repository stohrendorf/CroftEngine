IN_OUT ScreenPipelineInterface {
    vec2 texCoord;
    #ifdef SCREENSPRITE_TEXTURE
    flat float texIndex;
    #else
    flat vec4 topLeft;
    flat vec4 topRight;
    flat vec4 bottomLeft;
    flat vec4 bottomRight;
    #endif
} spi;
