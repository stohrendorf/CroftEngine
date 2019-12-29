#include "vtx_input.glsl"
#include "transform_interface.glsl"
#include "camera_interface.glsl"

void main() {
    gl_Position = u_projection * u_view * u_modelMatrix * vec4(a_position, 1);
}
