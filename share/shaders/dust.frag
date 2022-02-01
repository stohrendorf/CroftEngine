layout(location=0) out vec4 out_color;
layout(location=0) uniform vec3 u_baseColor;

in DustFSInterface {
    flat float lifetime;
} fs;

const float Alpha0 = 0.1;
const float Alpha1 = 0.3;

void main()
{
    float a = min(min(fs.lifetime, 1-fs.lifetime) * 3, 1);
    out_color = vec4(u_baseColor, a * 0.3);
}
