#include "flat_pipeline_interface.glsl"
#include "fx_input.glsl"

#ifdef AO
layout(bindless_sampler) uniform sampler2D u_ao;
#endif
#ifdef EDGES
layout(bindless_sampler) uniform sampler2D u_edges;
#endif

void main()
{
    out_color =
    #ifdef AO
    texture(u_ao, fpi.texCoord).r *
    #endif
    #ifdef EDGES
    texture(u_edges, fpi.texCoord).r *
    #endif
    texture(u_input, fpi.texCoord).rgb;
}
