#include "noise.glsl"
#include "time_uniform.glsl"
#include "flat_pipeline_interface.glsl"
#include "fx_input.glsl"

vec3 crt(in vec2 uv)
{
    const float NoiseIntensity = 0.01;

    vec3 col;
    col.r = texture(u_input, vec2(uv.x+0.001, uv.y+0.001)).r;
    col.g = texture(u_input, vec2(uv.x+0.000, uv.y-0.002)).g;
    col.b = texture(u_input, vec2(uv.x-0.002, uv.y+0.000)).b;

    float vig = 16.0 * fpi.texCoord.x * fpi.texCoord.y * (1.0-fpi.texCoord.x) * (1.0-fpi.texCoord.y);
    col *= vec3(pow(vig, 0.3));
    col *= vec3(0.95, 1.05, 0.95)*1.5;
    col *= 1.0 - 0.65 * vec3(clamp((mod(fpi.texCoord.x * textureSize(u_input, 0).x, 2.0)-1.0)*2.0, 0.0, 1.0));

    float noiseTime = TimeSeconds * 0.5;
    float noiseValue = texture(u_noise, vec2(1, 2*cos(noiseTime)) * noiseTime * 8.0 + uv * 2.0).x;

    return col +  noiseValue*noiseValue * NoiseIntensity;
}

void main()
{
    out_color = crt(fpi.texCoord);
}
