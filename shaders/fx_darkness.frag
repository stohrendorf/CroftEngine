uniform sampler2D u_texture;
uniform sampler2D u_depth;
uniform mat4 u_projection;

varying vec2 v_texCoord;

out vec4 out_color;

#ifdef WATER
    const float Frq1 = 12.6;
    const float TimeMult1 = 0.005;
    const float Amplitude1 = 0.00175;

    const float Frq2 = 19.7;
    const float TimeMult2 = 0.002;
    const float Amplitude2 = 0.0125;

    uniform float u_time;
#endif

#ifdef LENS_DISTORTION
    uniform float aspect_ratio;
    uniform float distortion_power;

float getStationaryRadius(in vec2 ctr) {
    if (distortion_power > 0.0)
        return length(ctr);

    if (aspect_ratio < 1.0)
        return ctr.x;
    else
        return ctr.y;
}

vec2 fisheye(in vec2 polar, in float stationary_radius) {
    float polar_dist = length(polar);
    float power = abs(distortion_power);
    return normalize(polar) * tan(polar_dist * power) * stationary_radius / tan(stationary_radius * power);
}

// same as fisheye, except that atan is used instead of tan
vec2 antiFisheye(in vec2 polar, in float stationary_radius) {
    float polar_dist = length(polar);
    float power = abs(distortion_power);
    return normalize(polar) * atan(polar_dist * power) * stationary_radius / atan(stationary_radius * power);
}
#endif

float depthAt(in vec2 uv)
{
    vec4 clipSpaceLocation;
    clipSpaceLocation.xy = uv * 2 - 1;
    clipSpaceLocation.z = texture2D(u_depth, uv).r * 2 - 1;
    clipSpaceLocation.w = 1;
    vec4 camSpaceLocation = inverse(u_projection) * clipSpaceLocation;
    float d = length(camSpaceLocation.xyz / camSpaceLocation.w);
    d /= 20480;
    return clamp(pow(d, 0.9), 0, 1);
}

void main()
{
    vec2 uv = v_texCoord;

#ifdef LENS_DISTORTION
    // normalized center coords
    vec2 norm_ctr = vec2(0.5, 0.5 / aspect_ratio);
    
    vec2 polar = uv - norm_ctr;
    float polar_dist = length(polar);

    float stationary_radius = getStationaryRadius(norm_ctr);

    if (distortion_power > 0.0)
        uv = norm_ctr + fisheye(polar, stationary_radius);
    else if (distortion_power < 0.0)
        uv = norm_ctr + antiFisheye(polar, stationary_radius);
#endif

#ifdef WATER
    float depth = depthAt(uv);

    float time = u_time * TimeMult1;
    vec2 angle = time + uv * Frq1;
    
    uv += vec2(sin(angle.x), sin(angle.y)) * Amplitude1 * (depth + 0.1);

    time = u_time * TimeMult2;
    angle = time + uv * Frq2;

    uv += vec2(sin(angle.x), sin(angle.y)) * Amplitude2 * (depth + 0.1);
#endif

    out_color = texture(u_texture, uv) * (1-depthAt(uv));
}
