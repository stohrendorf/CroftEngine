attribute vec4 a_position;
attribute vec4 a_normal;
attribute vec4 a_color;

uniform mat4 u_worldViewProjectionMatrix;
uniform float u_brightness;
uniform vec3 u_lightPosition;

varying vec4 v_color;

void main()
{
    gl_Position = u_worldViewProjectionMatrix * a_position;
    v_color = a_color;
}
