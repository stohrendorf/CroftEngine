uniform vec3 u_ambientColor;

uniform sampler2D u_diffuseTexture;

varying vec2 v_texCoord;

#ifdef HAS_VCOLOR
    varying vec3 v_color;
#endif

void main()
{
    vec4 baseColor = texture2D(u_diffuseTexture, v_texCoord);

    if(baseColor.a < 0.5)
        discard;

#ifdef HAS_VCOLOR
    gl_FragColor.r = baseColor.r * v_color.r;
    gl_FragColor.g = baseColor.g * v_color.g;
    gl_FragColor.b = baseColor.b * v_color.b;
    gl_FragColor.a = baseColor.a;
#else
    gl_FragColor = baseColor;
#endif
}
