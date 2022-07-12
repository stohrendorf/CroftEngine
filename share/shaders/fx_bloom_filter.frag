#include "flat_pipeline_interface.glsl"
#include "fx_input.glsl"
#include "util.glsl"

void main()
{
    const float Offset = 0.0;

    vec3 base = texture(u_input, fpi.texCoord).rgb;
    float baseLum = clamp((luminanceRgb(base) - Offset) / (1.0 - Offset), 0.0, 1.0);
    out_color = base * pow(baseLum, 2.0);
}
