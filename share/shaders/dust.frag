layout(location=0) out vec4 out_color;

in DustFSInterface {
    flat float lifetime;
} fs;

const vec3 BaseColor = vec3(1, 0.9, 0.8)*0.5;
const float Alpha0 = 0.1;
const float Alpha1 = 0.3;

void main()
{
    float a = min(min(fs.lifetime, 1-fs.lifetime) * 3, 1);
    out_color = vec4(BaseColor, a * 0.3);
}
