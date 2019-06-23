attribute vec3 a_position;

uniform mat4 u_mvp;

out vec3 v_vertexPosWorld;

void main()
{
    v_vertexPosWorld = a_position;
    gl_Position = u_mvp * vec4(a_position, 1);
}
