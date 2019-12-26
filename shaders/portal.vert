#include "vtx_input.glsl"
#include "portal_pipeline_interface.glsl"

uniform mat4 u_mvp;

void main()
{
    ppi.vertexPosWorld = a_position;
    gl_Position = u_mvp * vec4(a_position, 1);
}
