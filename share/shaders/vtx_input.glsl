layout(location=0) in vec3 a_position;

#ifdef VTX_INPUT_NORMAL
layout(location=1) in vec3 a_normal;
#endif

#ifdef VTX_INPUT_TEXCOORD
layout(location=2) in vec4 a_color;
layout(location=3) in vec2 a_texCoord;
layout(location=4) in float a_texIndex;

#ifdef VTX_INPUT_TEXCOORD_QUAD
layout(location=6) in float a_isQuad;
layout(location=7) in vec3 a_quadVert1;
layout(location=8) in vec3 a_quadVert2;
layout(location=9) in vec3 a_quadVert3;
layout(location=10) in vec3 a_quadVert4;
layout(location=11) in vec2 a_quadUv1;
layout(location=12) in vec2 a_quadUv2;
layout(location=13) in vec2 a_quadUv3;
layout(location=14) in vec2 a_quadUv4;
#endif
#endif

#ifdef VTX_INPUT_BONE_INDEX
layout(location=5) in float a_boneIndex;
#endif

#ifdef VTX_INPUT_COLOR_QUAD
#ifdef VTX_INPUT_TEXCOORD_QUAD
#error "Cannot use quad texcoord with color quad"
#endif
layout(location=6) in vec4 a_colorTopLeft;
layout(location=7) in vec4 a_colorTopRight;
layout(location=8) in vec4 a_colorBottomLeft;
layout(location=9) in vec4 a_colorBottomRight;
#endif
