uniform vec4 u_diffuseColor;

uniform float u_brightness;
uniform float u_ambient;
uniform vec3 u_lightPosition;

out vec4 out_color;

void main()
{
    out_color = u_diffuseColor;

    if(u_brightness > 0)
    {
        out_color *= (u_ambient + u_brightness);
        out_color.a = 1;
    }
}
