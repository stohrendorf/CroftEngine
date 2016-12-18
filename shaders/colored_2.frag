uniform vec4 u_diffuseColor;

uniform float u_brightness;
uniform vec3 u_lightPosition;

out vec4 out_color;

varying vec4 v_color;

void main()
{
    vec4 color;
    color.r = u_diffuseColor.r * v_color.r;
    color.g = u_diffuseColor.g * v_color.g;
    color.b = u_diffuseColor.b * v_color.b;
    color.a = u_diffuseColor.a * v_color.a;

    out_color = color * u_brightness;
    out_color.a = 1;
}
