#include "vtx_input.glsl"
#include "portal_pipeline_interface.glsl"
#include "camera_interface.glsl"

void main()
{
    ppi.vertexPosWorld = a_position;
    ppi.vertexPosView = (u_view * vec4(a_position, 1)).xyz;
    gl_Position = u_viewProjection * vec4(a_position, 1);
}
