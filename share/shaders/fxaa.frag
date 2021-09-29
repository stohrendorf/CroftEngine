layout(bindless_sampler) uniform sampler2D u_input;
vec2 inputSize = textureSize(u_input, 0);

#include "flat_pipeline_interface.glsl"
#include "time_uniform.glsl"

layout(location=0) out vec4 out_color;

#include "util.glsl"

#include "fxaa.glsl"

void main()
{
    out_color = vec4(fxaa(u_input, fpi.texCoord, 0.75, 0.166, 0.0833), 1.0);
}
