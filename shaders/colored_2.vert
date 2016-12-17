attribute vec4 a_position;
attribute vec4 a_normal;

uniform mat4 u_worldViewProjectionMatrix;
uniform float u_brightness;
uniform float u_ambient;
uniform vec3 u_lightPosition;

void main()
{
    gl_Position = u_worldViewProjectionMatrix * a_position;
}
