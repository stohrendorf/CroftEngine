uniform sampler2D u_texture;

vec2 screenSize = textureSize(u_texture, 0);

in vec2 v_texCoord;

layout(location=0) out vec4 out_color;

#include "util.glsl"

#include "fxaa.glsl"

void main()
{
    out_color.rgb = fxaa(0.75, 0.166, 0.0833);
    out_color.a = 1;
}
