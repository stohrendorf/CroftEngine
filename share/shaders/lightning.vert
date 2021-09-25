#include "vtx_input.glsl"
#include "transform_interface.glsl"
#include "camera_interface.glsl"

void main() {
    gl_Position = camera.viewProjection * modelTransform.m * vec4(a_position, 1);
}
