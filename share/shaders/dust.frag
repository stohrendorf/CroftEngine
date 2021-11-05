layout(location=0) out vec4 out_color;

in DustFSInterface {
    flat float lifetime;
} fs;

const vec3 BaseColor = vec3(1, 0.9, 0.8);
const vec3 Color0 = 0.1 * BaseColor;
const vec3 Color1 = 0.3 * BaseColor;

void main()
{
    float a = min(min(fs.lifetime, 1-fs.lifetime) * 4, 1);
    out_color = vec4(mix(Color0, Color1, a), 1.0);
}
