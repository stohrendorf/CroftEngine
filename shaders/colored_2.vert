attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec3 a_color;

uniform mat4 u_modelMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_projectionMatrix;

out vec3 v_color;
out vec3 v_vertexPos;
out vec3 v_normal;

void main()
{
    gl_Position = u_projectionMatrix * u_modelViewMatrix * vec4(a_position, 1);
    v_color = a_color;

    v_vertexPos = (u_modelMatrix * vec4(a_position, 1)).xyz;
    v_normal = normalize((u_modelMatrix * vec4(a_normal, 0)).xyz);
}
