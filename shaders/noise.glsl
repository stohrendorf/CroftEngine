uniform sampler2D u_noise;

float noise(vec2 p)
{
    float a = texture(u_noise, p*2.1).r;
    float b = texture(u_noise, p*3.17 + vec2(.3, .9)*p.xx).r;
    float c = texture(u_noise, p*9.87 + vec2(.1, .7)*p.yy).r;
    float d = texture(u_noise, p*9.87 + vec2(.6, .2)*p.yx).r;
    return (a+b+c+d) * 0.5 - 1.0;
}

float snoise(vec2 p)
{
    float a = texture(u_noise, p*3.17 + vec2(.3, .9)*p.yx).r;
    return a * 2.0 - 1.0;
}

vec2 noise2(vec2 p)
{
    return vec2(
    noise(p.xy),
    noise(p.yx + vec2(1.0))
    );
}
