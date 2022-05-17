layout(location=0) out vec4 out_color;
layout(location=0) uniform vec3 u_baseColor;

#include "util.glsl"

in DustFSInterface {
    float alpha;
} fs;

void main()
{
    out_color = vec4(toLinear(u_baseColor)*fs.alpha, fs.alpha);
}
