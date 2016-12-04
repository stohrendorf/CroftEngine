uniform sampler2D u_texture;
uniform sampler2D u_depth;
uniform mat4 u_projection;

varying vec2 v_texCoord;

out vec4 out_color;

#ifdef WATER
    const float Frq1 = 0.126;
    const float TimeMult1 = 0.005;
    const float Amplitude1 = 0.00175;

    const float Frq2 = 0.0197;
    const float TimeMult2 = 0.002;
    const float Amplitude2 = 0.0125;

    uniform float u_time;
#endif

float depth(vec2 uv)
{
    vec4 clipSpaceLocation;
    clipSpaceLocation.xy = uv * 2 - 1;
    clipSpaceLocation.z = texture(u_depth, uv).r * 2 - 1;
    clipSpaceLocation.w = 1;
    vec4 camSpaceLocation = inverse(u_projection) * clipSpaceLocation;
    float d = length(camSpaceLocation.xyz / camSpaceLocation.w);
    d /= 20480;
    return clamp(pow(d, 0.9), 0, 1);
}

void main()
{
    vec2 uv = v_texCoord;

#ifdef WATER
    float d = depth(uv);

    float time = u_time * TimeMult1;
    float xAngle = time + gl_FragCoord.y * Frq1;
    float yAngle = time + gl_FragCoord.x * Frq1;

    uv += vec2(sin(xAngle), sin(yAngle)) * Amplitude1 * (d + 0.1);

    time = u_time * TimeMult2;
    xAngle = time + gl_FragCoord.y * Frq2;
    yAngle = time + gl_FragCoord.x * Frq2;

    uv += vec2(sin(xAngle), sin(yAngle)) * Amplitude2 * (d + 0.1);
#endif

    out_color = texture(u_texture, uv) * (1-depth(uv));
}
