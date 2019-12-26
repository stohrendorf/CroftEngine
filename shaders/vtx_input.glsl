layout(location=0) in vec3 a_position;

#ifdef VTX_INPUT_NORMAL
layout(location=1) in vec3 a_normal;
#endif

#ifdef VTX_INPUT_COLOR
layout(location=2) in vec3 a_color;
#endif

#ifdef VTX_INPUT_TEXCOORD
layout(location=3) in vec2 a_texCoord;
#endif
