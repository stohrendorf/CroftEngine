#include "vtx_input.glsl"
#include "portal_pipeline_interface.glsl"
#include "camera_interface.glsl"

void main()
{
    ppi.vertexPosWorld = a_position;
    ppi.vertexPosView = (camera.view * vec4(a_position, 1)).xyz;
    gl_Position = camera.viewProjection * vec4(a_position, 1);
}
