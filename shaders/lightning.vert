layout(location=0) in vec3 a_position;

uniform mat4 u_modelViewMatrix;
uniform mat4 u_camProjection;

void main() {
    gl_Position = u_camProjection * u_modelViewMatrix * vec4(a_position, 1);
}
