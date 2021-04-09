uniform sampler2D u_noise;

vec3 gauss_noise(vec2 p)
{
    vec3 a = texture(u_noise, p*2.1).xyz;
    vec3 b = texture(u_noise, p*3.17 + vec2(.3, .9)*p.xx).xyz;
    vec3 c = texture(u_noise, p*9.87 + vec2(.1, .7)*p.yy).xyz;
    vec3 d = texture(u_noise, p*9.87 + vec2(.6, .2)*p.yx).xyz;
    return (a+b+c+d) * 0.5 - vec3(1.0);
}

vec3 basic_noise(vec2 p)
{
    vec3 a = texture(u_noise, p*3.17 + vec2(.3, .9)*p.yx).xyz;
    return a * 2.0 - vec3(1.0);
}

float noise(vec2 p)
{
    return gauss_noise(p).x;
}

float snoise(vec2 p)
{
    return basic_noise(p).x;
}

vec2 noise2(vec2 p)
{
    return gauss_noise(p).xy;
}

vec3 snoise3(vec3 p)
{
    return basic_noise(p.xy + p.zx*vec2(2.3145, 9.452) + p.yz*vec2(0.3252, 6.3254829)).xyz;
}
