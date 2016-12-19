uniform sampler2D u_diffuseTexture;

uniform float u_brightness;
uniform vec3 u_diffuseColor;

// varying vec2 v_texCoord;
varying vec3 v_color;
varying float v_shadeFactor;

out vec4 out_color;

void main()
{
    vec4 color;
    color.r = u_diffuseColor.r * v_color.r;
    color.g = u_diffuseColor.g * v_color.g;
    color.b = u_diffuseColor.b * v_color.b;
    color.a = 1;

    out_color = color * u_brightness;
    out_color += color * v_shadeFactor;
    out_color.a = 1;
}
