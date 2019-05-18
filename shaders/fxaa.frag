uniform sampler2D u_texture;
uniform vec2 u_screenSize;

in vec2 v_texCoord;

out vec4 out_color;

#include "util.glsl"

#include "fxaa.glsl"

void main()
{
    out_color.rgb = fxaa(0.75, 0.166, 0.0833);
    out_color.a = 1;
}
