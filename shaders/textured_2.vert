attribute vec4 a_position;
attribute vec4 a_normal;
attribute vec2 a_texCoord;

uniform mat4 u_worldViewProjectionMatrix;
uniform float u_brightness;
uniform vec3 u_lightPosition;

varying vec2 v_texCoord;

attribute vec3 a_color;
varying vec3 v_color;

void main()
{
    gl_Position = u_worldViewProjectionMatrix * a_position;
    v_texCoord = a_texCoord;
    v_color = a_color;
}
