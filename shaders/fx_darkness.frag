uniform sampler2D u_texture;
uniform sampler2D u_depth;
uniform mat4 u_projection;

varying vec2 v_texCoord;

out vec4 out_color;

float depth()
{
    vec4 clipSpaceLocation;
    clipSpaceLocation.xy = v_texCoord * 2 - 1;
    clipSpaceLocation.z = texture(u_depth, v_texCoord).r * 2 - 1;
    clipSpaceLocation.w = 1;
    vec4 camSpaceLocation = inverse(u_projection) * clipSpaceLocation;
    float d = length(camSpaceLocation.xyz / camSpaceLocation.w);
    d /= 20480;
    return clamp(1 - pow(d, 0.9), 0, 1);
}

void main()
{
    out_color = texture(u_texture, v_texCoord) * depth();
}
