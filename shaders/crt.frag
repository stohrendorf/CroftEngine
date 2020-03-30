#include "util.glsl"
#include "time_uniform.glsl"
#include "flat_pipeline_interface.glsl"

layout(binding=0) uniform sampler2D u_input;
layout(binding=1) uniform sampler2D u_noise;
layout(location=0) out vec4 out_color;

vec3 crt(in sampler2D tex, in vec2 uv, in float time)
{
    float x = sin(0.3*time+uv.y*21.0) * sin(0.7*time+uv.y*29.0) * sin(0.3+0.33*time+uv.y*31.0) * 0.0017;

    vec3 col;
    col.r = texture(tex, vec2(x+uv.x+0.001, uv.y+0.001)).r;
    col.g = texture(tex, vec2(x+uv.x+0.000, uv.y-0.002)).g;
    col.b = texture(tex, vec2(x+uv.x-0.002, uv.y+0.000)).b;
    col.r += 0.08 * texture(tex, 0.75*vec2(x+0.025, -0.027)+vec2(uv.x+0.001, uv.y+0.001)).r;
    col.g += 0.05 * texture(tex, 0.75*vec2(x+-0.022, -0.02)+vec2(uv.x+0.000, uv.y-0.002)).g;
    col.b += 0.08 * texture(tex, 0.75*vec2(x+-0.02, -0.018)+vec2(uv.x-0.002, uv.y+0.000)).b;

    float vig = 16.0 * uv.x * uv.y * (1.0-uv.x) * (1.0-uv.y);
    col *= vec3(pow(vig, 0.3));

    col *= vec3(0.95, 1.05, 0.95);

    float scans = 0.35 * (1 + sin(3.5 * time + uv.y * textureSize(u_input, 0).y * 1.5));
    col *= vec3(0.7 + 0.7 * pow(scans, 1.7));
    col *= 1.0 + 0.01 * sin(110.0 * time);
    col *= 1.0 - 0.65 * vec3(clamp((mod(fpi.texCoord.x * textureSize(u_input, 0).x, 2.0)-1.0)*2.0, 0.0, 1.0));

    return col;
}

float noise(in vec2 p, in float time)
{
    float value = texture(u_noise, vec2(1, 2*cos(time)) * time * 8 + p).x;
    return value*value;
}

void main()
{
    float time = time_seconds()/2;
    vec3 video = crt(u_input, fpi.texCoord, time);

    const float Intensity = 0.01;
    video += noise(fpi.texCoord * 2, time) * Intensity;

    out_color.rgb = video;
    out_color.a = 1;
}
