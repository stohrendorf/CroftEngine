layout(location=0) in vec3 a_position;

layout(std140) uniform Transform {
    mat4 u_modelMatrix;
    mat4 u_modelViewMatrix;
    mat4 u_camProjection;
};

void main() {
    gl_Position = u_camProjection * u_modelViewMatrix * vec4(a_position, 1);
}
