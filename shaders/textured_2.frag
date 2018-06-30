uniform sampler2D u_diffuseTexture;

in vec2 v_texCoord;
in vec3 v_color;
in vec3 v_vertexPos;
in vec3 v_normal;

out vec4 out_color;

#include "lighting.inc.frag"

void main()
{
    vec4 baseColor = texture2D(u_diffuseTexture, v_texCoord);

    if(baseColor.a < 0.5)
        discard;

    out_color.r = baseColor.r * v_color.r;
    out_color.g = baseColor.g * v_color.g;
    out_color.b = baseColor.b * v_color.b;
    out_color.a = baseColor.a;

#ifdef WATER
    const vec4 WaterColor = vec4(149 / 255.0f, 229 / 255.0f, 229 / 255.0f, 1);

    out_color *= WaterColor;
#endif

    out_color *= calcShadeFactor(v_normal, v_vertexPos);
    out_color.a = 1;
}
