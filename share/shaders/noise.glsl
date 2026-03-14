layout(bindless_sampler) uniform sampler2D u_noise;

// noise functions return values in the range -1..1

vec3 noise_gaussian(vec2 p)
{
    vec4 n1 = texture(u_noise, p * 2.1);
    vec4 n2 = texture(u_noise, p * 3.17 + vec2(0.3, 0.9) * p.x);
    vec4 n3 = texture(u_noise, p * 9.87 + vec2(0.1, 0.7) * p.y);

    // Each component is a sum of 4 independent samples (Irwin-Hall distribution)
    return vec3(
    n1.r + n2.r + n3.r + n1.a,
    n1.g + n2.g + n3.g + n2.a,
    n1.b + n2.b + n3.b + n3.a
    ) * 0.5 - 1.0;
}

vec3 basic_noise(vec2 p)
{
    vec2 st = p + vec2(58.3, 66.4) * p.yx;
    return texture(u_noise, st).xyz * 2.0 - 1.0;
}

float noise(vec2 p)
{
    return noise_gaussian(p).x;
}

float snoise(vec2 p)
{
    return basic_noise(p).x;
}

vec2 noise2(vec2 p)
{
    return noise_gaussian(p).xy;
}

vec3 snoise3(vec3 p)
{
    return basic_noise(p.xy + p.zx*vec2(2.3145, 9.452) + p.yz*vec2(0.3252, 6.3254829)).xyz;
}
