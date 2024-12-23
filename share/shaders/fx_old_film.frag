// based on https://www.shadertoy.com/view/Wdj3zV

#include "flat_pipeline_interface.glsl"
#include "fx_input.glsl"
#include "util.glsl"
#include "time_uniform.glsl"
#include "noise.glsl"

const float SEQUENCE_LENGTH = 24.0;
const float FPS = 12.0;

vec4 vignette(vec2 uv, in float time)
{
    uv *= 1.0 - uv.yx;
    float vig = uv.x * uv.y * 15.0;
    float t = sin(time * 23.0) * cos(time * 8.0 + 0.5);
    vig = pow(vig, 0.4 + t * 0.05);
    return vec4(vig);
}

float easeIn(in float t0, in float t1, in float t)
{
    return 2.0 * smoothstep(t0, 2.0 * t1 - t0, t);
}

float filmDirt(in vec2 pp, in float time)
{
    vec2 nseLookup2 = pp + vec2(0.5, 0.9) * time;
    vec3 nse2 = gauss_noise(0.03 * nseLookup2) * 0.5 + 0.5;

    const float thresh = 0.6;
    const float aaRad = 0.1;
    vec3 mul = smoothstep(vec3(thresh-aaRad), vec3(thresh+aaRad), nse2);

    float seed = gauss_noise(time * vec2(0.35, 1.0)).x * 0.3 + 0.7;

    float result = clamp(0.0, 1.0, seed + 0.7) + 0.3 * smoothstep(0.0, SEQUENCE_LENGTH, time) + 0.06 * easeIn(19.2, 19.4, time);

    const float band = 0.05;
    if (0.3 < seed && seed < 0.3 + band) {
        return mul.x * result;
    }
    else if (0.6 < seed && seed < 0.6 + band) {
        return mul.y * result;
    }
    else if (0.9 < seed && seed < 0.9 + band) {
        return mul.z * result;
    }
    else {
        return result;
    }
}

float jumpCut(in float seqTime)
{
    const float jct1 = 7.7;
    const float jct2 = 8.2;

    float jc1 = step(jct1, seqTime);
    float jc2 = step(jct2, seqTime);

    return 0.8 * jc1 - (jc2 - jc1) * (seqTime - jct1) - 0.9 * jc2;
}

vec2 moveImage(in vec2 uv, in float time)
{
    return 0.002 * vec2(
    cos(time * 3.0) * sin(time * 12.0 + 0.25),
    sin(time * 1.0 + 0.5) * cos(time * 15.0 + 0.25)
    ) + uv;
}

void main()
{
    vec2 uv = fpi.texCoord;
    vec2 qq = 2.0 * uv + -1.0;

    vec2 res = textureSize(u_input, 0).xy;
    qq.x *= res.x / res.y;

    float time = float(int(mod(u_time/1000.0, SEQUENCE_LENGTH) * FPS)) / FPS;

    vec4 dirt = vec4(filmDirt(qq, time + jumpCut(time)));
    vec4 image = texture(u_input, moveImage(uv, time));
    vec4 vig = vignette(uv, time);

    out_color = vec3(luminanceRgb(image * dirt * vig));
}
