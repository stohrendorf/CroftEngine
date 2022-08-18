// based on https://www.shadertoy.com/view/XsjSzR

#include "flat_pipeline_interface.glsl"
#include "fx_input.glsl"
#include "util.glsl"
#include "constants.glsl"

const float Range = 0.25;
const float Brightness = BRIGHTNESS/100.0 * Range;
const float Contrast = CONTRAST/100.0 * Range;

const float Slope = max(tan((Contrast+1.0)*PI/4.0), 0.0);
const float Intercept = Brightness + ((1.0 - Brightness)/2.0) * (1.0 - Slope);

void main()
{
    vec3 c = texture(u_input, fpi.texCoord).rgb;
    out_color = Slope*c + vec3(Intercept);
}
