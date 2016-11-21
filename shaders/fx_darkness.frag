uniform sampler2D u_texture;
uniform sampler2D u_depth;

varying vec2 v_texCoord;

out vec4 out_color;

float depth()
{
    float d = texture(u_depth, v_texCoord).r;
    d -= 10.0 / 20;
    d /= 10.0 / 20;
    return clamp(1 - pow(d, 512), 0, 1);
}

void main()
{
    out_color = texture2D(u_texture, v_texCoord) * depth();
}
