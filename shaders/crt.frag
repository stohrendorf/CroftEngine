#include "util.glsl"
#include "time_uniform.glsl"
#include "flat_pipeline_interface.glsl"

layout(binding=0) uniform sampler2D u_input;
layout(binding=1) uniform sampler2D u_noise;
layout(location=0) out vec4 out_color;

vec3 crt(in sampler2D tex, in vec2 uv)
{
    vec3 col;
    col.r = texture(tex, vec2(uv.x+0.001, uv.y+0.001)).r;
    col.g = texture(tex, vec2(uv.x+0.000, uv.y-0.002)).g;
    col.b = texture(tex, vec2(uv.x-0.002, uv.y+0.000)).b;

    float vig = 16.0 * uv.x * uv.y * (1.0-uv.x) * (1.0-uv.y);
    col *= vec3(pow(vig, 0.3));

    col *= vec3(0.95, 1.05, 0.95)*1.5;

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
    float time = time_seconds()/2.0;
    vec3 color = crt(u_input, fpi.texCoord);

    const float Intensity = 0.01;
    color += noise(fpi.texCoord * 2, time) * Intensity;

    out_color = vec4(color, 1.0);
}
