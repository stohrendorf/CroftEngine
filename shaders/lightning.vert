#include "vtx_input.glsl"
#include "transform_interface.glsl"

void main() {
    gl_Position = u_camProjection * u_modelViewMatrix * vec4(a_position, 1);
}
