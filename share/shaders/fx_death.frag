#include "noise.glsl"
#include "time_uniform.glsl"
#include "flat_pipeline_interface.glsl"
#include "fx_input.glsl"
#include "util.glsl"

layout(location=1) uniform float u_strength;

void main()
{
    vec3 col = texture(u_input, fpi.texCoord).rgb;
    float lum = luminance(col);

    float vig = fpi.texCoord.x * fpi.texCoord.y * (1.0-fpi.texCoord.x) * (1.0-fpi.texCoord.y);
    lum *= sqrt(vig*2);

    vec3 monochrome = vec3(lum, lum, lum);

    out_color = mix(col, monochrome, clamp(u_strength, 0, 1));
}
