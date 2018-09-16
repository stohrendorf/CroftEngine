uniform sampler2D u_texture;
uniform sampler2D u_depth;
uniform mat4 u_projection;

in vec2 v_texCoord;

out vec4 out_color;

const float PI = 3.14159265;
const float Z_max = 20480;

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
#endif

float depth_at(in vec2 uv)
{
    vec4 clipSpaceLocation;
    clipSpaceLocation.xy = uv * 2 - vec2(1); // normalized device coordinates
    clipSpaceLocation.z = texture2D(u_depth, uv).r * 2.0 - 1.0;
    clipSpaceLocation.w = 1;
    vec4 camSpaceLocation = inverse(u_projection) * clipSpaceLocation;
    float d = length(camSpaceLocation.xyz / camSpaceLocation.w);
    d /= Z_max;
    return clamp(d, 0.0, 1.0);
}

// autofocus
float dof_focal_depth = depth_at(vec2(0.5, 0.5));

float brightness(in float depth)
{
    return 1.0 - depth;
}

vec3 shaded_texel(in vec2 uv, in float depth)
{
    return texture2D(u_texture, uv).rgb * brightness(depth);
}

#define DOF

#ifdef DOF
#ifdef WATER
const float dof_scale = 200.0 / Z_max;
const float dof_offset = 80.0 / Z_max;
#else
const float dof_scale = 40.0 / Z_max;
const float dof_offset = 20.0 / Z_max;
#endif

// get a random 2D-vector, each component within -1..1
vec2 rand2(in vec2 uv)
{
    float noise_x = clamp(fract(sin(dot(uv, vec2(12.9898,78.233)    )) * 43758.5453), 0.0, 1.0)*2.0-1.0;
    float noise_y = clamp(fract(sin(dot(uv, vec2(12.9898,78.233)*2.0)) * 43758.5453), 0.0, 1.0)*2.0-1.0;
    return vec2(noise_x, noise_y);
}

float dof_focal_point_distance(in float depth)
{
    return clamp(abs(depth - dof_focal_depth) / dof_focal_depth, 0.0, 1.0);
}

float dof_blur_radius(in float depth) {
    return dof_focal_point_distance(depth) * dof_scale + dof_offset;
}
#endif

#ifdef LENS_DISTORTION
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
#endif

#ifdef WATER
void do_water_distortion_frq(inout vec2 uv, in float timeMult, in float frq, in float amplitude)
{
    vec2 phase = vec2(u_time * timeMult) + uv * frq;
    
    uv += vec2(sin(phase.x), sin(phase.y)) * amplitude;
}

void do_water_distortion(inout vec2 uv)
{
    do_water_distortion_frq(uv, TimeMult1, Frq1, Amplitude1);
    do_water_distortion_frq(uv, TimeMult2, Frq2, Amplitude2);
}
#endif

#ifdef DOF
vec3 do_dof(in vec2 uv)
{
    float depth = depth_at(uv);

    const int dof_blends = 6;
    const int dof_rings = 6;
    const float angle_step = PI*2.0 / dof_blends;

    float dist_step = dof_blur_radius(depth) / dof_rings;

    vec3 sample_color = shaded_texel(uv, depth);
    float sample_weight_sum = 1.0;
    for (int i = 1; i <= dof_blends; i += 1)
    {   
        for (int j = 0; j < dof_rings; j += 1)   
        {
            vec2 p;
            p.x = sin(i*angle_step) * dist_step * j;
            p.y = cos(i*angle_step) * dist_step * j;
            vec2 r = rand2(uv + p) * dist_step / 2.0;
            vec2 peek = uv + p + r;
            float local_weight = 1.0 - smoothstep(j + 1.0, 0.0, dof_rings);
            sample_color += shaded_texel(peek, depth_at(peek)) * local_weight;
            sample_weight_sum += local_weight;
        }
    }
    sample_color /= sample_weight_sum;
    return sample_color;
}
#endif

void main()
{
    vec2 uv = v_texCoord;

#ifdef LENS_DISTORTION
    do_lens_distortion(uv);
#endif

#ifdef WATER
    do_water_distortion(uv);
#endif

#ifndef DOF
    out_color.rgb = shaded_texel(uv, depth_at(uv));
#else
    out_color.rgb = do_dof(uv);
#endif
    out_color.a = 1;
}
