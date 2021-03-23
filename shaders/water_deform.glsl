#include "time_uniform.glsl"

const float Frq1 = 12.6;
const float TimeMult1 = 0.002;
const float Amplitude1 = 0.001;

const float Frq2 = 19.7;
const float TimeMult2 = 0.001;
const float Amplitude2 = 0.0055;

void do_water_distortion_frq(inout vec2 uv, in float timeMult, in float scale, in float amplitude)
{
    vec2 phase = uv * scale + vec2(u_time * timeMult);

    uv += vec2(sin(phase.x), sin(phase.y)) * amplitude;
}

void do_water_distortion(inout vec2 uv)
{
    do_water_distortion_frq(uv, TimeMult1, Frq1, Amplitude1);
    do_water_distortion_frq(uv, TimeMult2, Frq2, Amplitude2);
}
