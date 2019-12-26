uniform sampler2D u_texture;
vec2 screenSize = textureSize(u_texture, 0);

#include "flat_pipeline_interface.glsl"

layout(location=0) out vec4 out_color;

#include "util.glsl"

#include "fxaa.glsl"

void main()
{
    out_color.rgb = fxaa(u_texture, fpi.texCoord, 0.75, 0.166, 0.0833);
    out_color.a = 1;
}
