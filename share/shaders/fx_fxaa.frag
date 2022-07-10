#include "flat_pipeline_interface.glsl"
#include "fx_input.glsl"

#include "util.glsl"

#include "fxaa.glsl"

void main()
{
    const float subpixQuality = 0.75;
    const float edgeThreshold = 0.125;
    const float edgeThresholdMin = 0.0625;
    out_color = fxaa(fpi.texCoord, u_input, subpixQuality, edgeThreshold, edgeThresholdMin).rgb;
}
