attribute vec4 a_position;
uniform mat4 u_worldViewProjectionMatrix;
uniform mat4 u_worldViewMatrix;

varying float v_depth;

void main()
{
    gl_Position = u_worldViewProjectionMatrix * a_position;

    vec4 cam = u_worldViewMatrix * a_position;
    v_depth = sqrt(cam.x * cam.x + cam.y * cam.y + cam.z * cam.z);
}
