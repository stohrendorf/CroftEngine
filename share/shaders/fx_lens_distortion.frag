#include "flat_pipeline_interface.glsl"
#include "camera_interface.glsl"
#include "time_uniform.glsl"
#include "fx_input.glsl"

float distortionPower = u_inWater != 0.0 ? -2.0 : -1.0;
float absDistortionPower = abs(distortionPower);

vec2 fisheye(in vec2 polar, in float stationary_radius)
{
    return normalize(polar) * tan(length(polar) * absDistortionPower) * stationary_radius / tan(stationary_radius * absDistortionPower);
}

// same as fisheye, except that atan is used instead of tan
vec2 anti_fisheye(in vec2 polar, in float stationary_radius)
{
    return normalize(polar) * atan(length(polar) * absDistortionPower) * stationary_radius / atan(stationary_radius * absDistortionPower);
}

vec2 do_lens_distortion(in vec2 uv)
{
    float stationary_radius = max(0.5, 0.5 / camera.aspectRatio);

    if (distortionPower > 0.0) {
        return vec2(0.5, 0.5) + fisheye(uv - vec2(0.5, 0.5), stationary_radius);
    }
    else if (distortionPower < 0.0) {
        return vec2(0.5, 0.5) + anti_fisheye(uv - vec2(0.5, 0.5), stationary_radius);
    }
    else {
        return uv;
    }
}

void main()
{
    out_color = texture(u_input, do_lens_distortion(fpi.texCoord)).rgb;
}
