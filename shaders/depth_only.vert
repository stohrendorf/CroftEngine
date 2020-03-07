#include "vtx_input.glsl"
#include "transform_interface.glsl"
#include "camera_interface.glsl"

void main()
{
    vec4 vtx = u_viewProjection * u_modelMatrix * vec4(a_position, 1);
    vtx.z *= 1.0 + 1.0/1024 * abs(vtx.w);// depth offset
    gl_Position = vtx;
}
