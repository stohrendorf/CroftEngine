uniform mat4 u_worldViewProjectionMatrix;

attribute vec4 a_position;

attribute vec2 a_texCoord;
varying vec2 v_texCoord;

#ifdef HAS_VCOLOR
    attribute vec3 a_color;
    varying vec3 v_color;
#endif

void main()
{
    gl_Position = u_worldViewProjectionMatrix * a_position;
    v_texCoord = a_texCoord;

#ifdef HAS_VCOLOR
    v_color = a_color;
#endif
}
