uniform sampler2D u_diffuseTexture;

uniform vec3 u_diffuseColor;

// varying vec2 v_texCoord;
varying vec3 v_color;
varying float v_shadeFactor;

out vec4 out_color;

void main()
{
    out_color.r = u_diffuseColor.r * v_color.r;
    out_color.g = u_diffuseColor.g * v_color.g;
    out_color.b = u_diffuseColor.b * v_color.b;
    out_color.a = 1;

    out_color *= v_shadeFactor;
    out_color.a = 1;
}
