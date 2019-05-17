// get a random 2D-vector, each component within -1..1
vec2 rand2(in vec2 uv)
{
    float noise_x = clamp(fract(sin(dot(uv, vec2(12.9898,78.233)    )) * 43758.5453), 0.0, 1.0)*2.0-1.0;
    float noise_y = clamp(fract(sin(dot(uv, vec2(12.9898,78.233)*2.0)) * 43758.5453), 0.0, 1.0)*2.0-1.0;
    return vec2(noise_x, noise_y);
}

float brightness(in float depth)
{
    return 1.0 - depth;
}

vec3 shade_texel(in vec3 rgb, in float depth)
{
    return rgb * brightness(depth);
}

vec3 shaded_texel(in vec2 uv, in float depth)
{
    return shade_texel(texture2D(u_texture, uv).rgb, depth);
}

float luminance(in vec3 color)
{
    return dot(color, vec3(0.212656, 0.715158, 0.072186));
}

float luminance(in vec4 color)
{
    return dot(vec3(color), vec3(0.212656, 0.715158, 0.072186));
}
