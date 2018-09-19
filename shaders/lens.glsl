uniform float aspect_ratio;
uniform float distortion_power;

vec2 fisheye(in vec2 polar, in float stationary_radius) {
    float power = abs(distortion_power);
    return normalize(polar) * tan(length(polar) * power) * stationary_radius / tan(stationary_radius * power);
}

// same as fisheye, except that atan is used instead of tan
vec2 anti_fisheye(in vec2 polar, in float stationary_radius) {
    float power = abs(distortion_power);
    return normalize(polar) * atan(length(polar) * power) * stationary_radius / atan(stationary_radius * power);
}

void do_lens_distortion(inout vec2 uv)
{
    float stationary_radius = max(0.5, 0.5 / aspect_ratio);

    if (distortion_power > 0.0)
        uv = vec2(0.5, 0.5) + fisheye(uv - vec2(0.5, 0.5), stationary_radius);
    else if (distortion_power < 0.0)
        uv = vec2(0.5, 0.5) + anti_fisheye(uv - vec2(0.5, 0.5), stationary_radius);
}
