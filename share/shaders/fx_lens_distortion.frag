#include "flat_pipeline_interface.glsl"
#include "camera_interface.glsl"
#include "time_uniform.glsl"
#include "fx_input.glsl"

float distortionPower = u_inWater != 0.0 ? -2.0 : -1.0;
float absDistortionPower = abs(distortionPower);

vec2 fisheye(in vec2 polar, in float stationary_radius)
{
    float l = length(polar);
    return polar * (tan(l * absDistortionPower) / (l * tan(stationary_radius * absDistortionPower))) * stationary_radius;
}

// same as fisheye, except that atan is used instead of tan
vec2 anti_fisheye(in vec2 polar, in float stationary_radius)
{
    float l = length(polar);
    return polar * (atan(l * absDistortionPower) / (l * atan(stationary_radius * absDistortionPower))) * stationary_radius;
}

vec2 do_lens_distortion(in vec2 uv)
{
    float stationary_radius = max(0.5, 0.5 / camera.aspectRatio);
    vec2 polar = uv - vec2(0.5, 0.5);

    if (distortionPower > 0.0) {
        return vec2(0.5, 0.5) + fisheye(polar, stationary_radius);
    }
    else if (distortionPower < 0.0) {
        return vec2(0.5, 0.5) + anti_fisheye(polar, stationary_radius);
    }
    else {
        return uv;
    }
}

void main()
{
    out_color = texture(u_input, do_lens_distortion(fpi.texCoord)).rgb;
}
