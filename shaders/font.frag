///////////////////////////////////////////////////////////
// Uniforms
uniform sampler2D u_texture;

///////////////////////////////////////////////////////////
// Varyings
varying vec2 v_texCoord;
varying vec4 v_color;


void main()
{
    vec4 orig = gl_FragColor;
    vec4 blendColor = v_color;
    blendColor.a = texture2D(u_texture, v_texCoord).a * v_color.a;

    gl_FragColor = mix(orig, blendColor, blendColor.a);
}
