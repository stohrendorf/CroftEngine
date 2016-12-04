uniform vec3 u_ambientColor;
uniform sampler2D u_diffuseTexture;

varying vec2 v_texCoord;

out vec4 out_color;

#ifdef HAS_VCOLOR
    varying vec3 v_color;
#endif

void main()
{
    vec4 baseColor = texture2D(u_diffuseTexture, v_texCoord);

    if(baseColor.a < 0.5)
        discard;

#ifdef HAS_VCOLOR
    out_color.r = baseColor.r * v_color.r;
    out_color.g = baseColor.g * v_color.g;
    out_color.b = baseColor.b * v_color.b;
    out_color.a = baseColor.a;
#else
    out_color = baseColor;
#endif

#ifdef WATER
    const vec4 WaterColor = vec4(149 / 255.0f, 229 / 255.0f, 229 / 255.0f, 1);

    out_color *= WaterColor;
#endif
}
