layout(location=0) in vec3 a_position;

uniform mat4 u_modelMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_camProjection;

void main()
{
    vec4 tmp = u_modelViewMatrix * vec4(a_position, 1);
    gl_Position = u_camProjection * tmp;
}
