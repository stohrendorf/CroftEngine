uniform vec3 u_diffuseColor;

in vec3 v_color;
in vec3 v_vertexPos;
in vec3 v_normal;

out vec4 out_color;

#include "lighting.inc.frag"

vec3 srgbEncode(in vec3 color){
   float r = color.r < 0.0031308 ? 12.92 * color.r : 1.055 * pow(color.r, 1.0/2.4) - 0.055;
   float g = color.g < 0.0031308 ? 12.92 * color.g : 1.055 * pow(color.g, 1.0/2.4) - 0.055;
   float b = color.b < 0.0031308 ? 12.92 * color.b : 1.055 * pow(color.b, 1.0/2.4) - 0.055;
   return vec3(r, g, b);
}

vec3 srgbDecode(in vec3 color){
   float r = color.r < 0.04045 ? (1.0 / 12.92) * color.r : pow((color.r + 0.055) * (1.0 / 1.055), 2.4);
   float g = color.g < 0.04045 ? (1.0 / 12.92) * color.g : pow((color.g + 0.055) * (1.0 / 1.055), 2.4);
   float b = color.b < 0.04045 ? (1.0 / 12.92) * color.b : pow((color.b + 0.055) * (1.0 / 1.055), 2.4);
   return vec3(r, g, b);
}

void main()
{
    vec3 color = srgbDecode(u_diffuseColor);

    out_color.r = color.r * v_color.r;
    out_color.g = color.g * v_color.g;
    out_color.b = color.b * v_color.b;

    out_color *= calcShadeFactor(v_normal, v_vertexPos);
    out_color.a = 1;
}
