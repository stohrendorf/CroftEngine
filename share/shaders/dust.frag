layout(location=0) out vec4 out_color;
layout(location=0) uniform vec3 u_baseColor;

#include "util.glsl"

in DustFSInterface {
    flat float lifetime;
} fs;

void main()
{
    float a = clamp(min(fs.lifetime, 1-fs.lifetime) * 3, 0, 1);
    out_color = vec4(toLinear(u_baseColor), a * 0.5);
}
