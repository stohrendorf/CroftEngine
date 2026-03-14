layout(location=0) out vec4 out_color;
layout(location=0) uniform vec3 u_baseColor;

#include "util.glsl"

in DustFSInterface {
    float alpha;
} fs;

void main()
{
    vec2 coord = gl_PointCoord - vec2(0.5);
    if (dot(coord, coord) > 0.25) {
        discard;
    }
    out_color = vec4(toLinear(u_baseColor) * fs.alpha, fs.alpha);
}
