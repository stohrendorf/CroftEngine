#include "flat_pipeline_interface.glsl"
#include "time_uniform.glsl"
#include "fx_input.glsl"

vec2 inputSize = textureSize(u_input, 0);
#include "util.glsl"

#include "fxaa.glsl"

void main()
{
    out_color = fxaa(u_input, fpi.texCoord, 0.75, 0.166, 0.0833);
}
