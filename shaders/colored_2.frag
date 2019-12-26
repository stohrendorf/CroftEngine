#include "geometry_pipeline_interface.glsl"

layout(location=0) out vec4 out_color;
layout(location=1) out vec3 out_normal;
layout(location=2) out vec3 out_position;

#include "lighting.glsl"

float srgbEncode(in float cl)
{
    if(cl<0)
        return 0;
    else if(cl < 0.0031308)
        return 12.92 * cl;
    else if(cl<1)
        return 1.055 * pow(cl, 0.41666) - 0.055;
    else
        return 1;
}

vec3 srgbEncode(in vec3 color)
{
   float r = srgbEncode(color.r);
   float g = srgbEncode(color.g);
   float b = srgbEncode(color.b);
   return vec3(r, g, b);
}

float srgbDecode(in float cs)
{
    if(cs <= 0.04045)
        return cs / 12.92;
    else
        return pow((cs + 0.055)/1.055, 2.4);
}

vec3 srgbDecode(in vec3 color)
{
   float r = srgbDecode(color.r);
   float g = srgbDecode(color.g);
   float b = srgbDecode(color.b);
   return vec3(r, g, b);
}

void main()
{
    out_color.rgb = srgbDecode(gpi.color) * calc_positional_lighting(gpi.normal, gpi.vertexPos) * shadow_map_multiplier();
    out_color.a = 1.0;

    out_normal = gpi.ssaoNormal;
    out_position = gpi.vertexPos;
}
