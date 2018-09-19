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

vec3 shaded_texel(in vec2 uv, in float depth)
{
    return texture2D(u_texture, uv).rgb * brightness(depth);
}
