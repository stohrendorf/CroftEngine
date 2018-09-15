uniform sampler2D u_texture;
uniform sampler2D u_depth;
uniform mat4 u_projection;

in vec2 v_texCoord;

out vec4 out_color;

const float PI = 3.14159265;
const float Z_max = 20480;
const float depth_darkness_near = 12.0 / 20;

#ifdef WATER
    const float Frq1 = 12.6;
    const float TimeMult1 = 0.005;
    const float Amplitude1 = 0.00175;

    const float Frq2 = 19.7;
    const float TimeMult2 = 0.002;
    const float Amplitude2 = 0.0125;

    uniform float u_time;
#endif

float depthAt(in vec2 uv)
{
    vec4 clipSpaceLocation;
    clipSpaceLocation.xy = uv * 2 - 1;
    clipSpaceLocation.z = texture2D(u_depth, uv).r * 2 - 1;
    clipSpaceLocation.w = 1;
    vec4 camSpaceLocation = inverse(u_projection) * clipSpaceLocation;
    float d = length(camSpaceLocation.xyz / camSpaceLocation.w);
    d /= Z_max;
    return clamp(d, 0, 1);
}

// autofocus
float dof_focal = depthAt(vec2(0.5, 0.5));

float darkness(in float depth)
{
    float f = clamp((depth - depth_darkness_near) / (1 - depth_darkness_near), 0, 1);
    return 1 - f * f * (3 - 2 * f);
}

vec3 darkness_texel(in vec2 uv, in float depth)
{
    return texture2D(u_texture, uv).rgb * darkness(depth);
}

#define DOF

#ifdef DOF
#ifdef WATER
const float dof_sensor_size = 200.0 / Z_max;
const float dof_center_size = 80.0 / Z_max;
#else
const float dof_sensor_size = 40.0 / Z_max;
const float dof_center_size = 20.0 / Z_max;
#endif

// get a random 2D-vector, each component within -1..1
vec2 dofRand(in vec2 uv)
{
    float noiseX = clamp(fract(sin(dot(uv, vec2(12.9898,78.233)    )) * 43758.5453), 0.0, 1.0)*2.0-1.0;
    float noiseY = clamp(fract(sin(dot(uv, vec2(12.9898,78.233)*2.0)) * 43758.5453), 0.0, 1.0)*2.0-1.0;
    return vec2(noiseX, noiseY);
}

float calcDofFactor(in float depth)
{
    return clamp(abs(depth - dof_focal) / dof_focal, 0.0, 1.0);
}

float calcDofBlurRadius(in float depth) {
    float d = calcDofFactor(depth);
    return d * dof_sensor_size + dof_center_size;
}
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

void doLensDistortion(inout vec2 uv)
{
    // normalized center coords
    vec2 norm_ctr = vec2(0.5, 0.5 / aspect_ratio);
    
    vec2 polar = uv - norm_ctr;
    float polar_dist = length(polar);

    float stationary_radius = getStationaryRadius(norm_ctr);

    if (distortion_power > 0.0)
        uv = norm_ctr + fisheye(polar, stationary_radius);
    else if (distortion_power < 0.0)
        uv = norm_ctr + antiFisheye(polar, stationary_radius);
}
#endif

#ifdef WATER
void doWaterFrq(inout vec2 uv, in float timeMult, in float frq, in float amplitude)
{
    vec2 phase = vec2(u_time * timeMult) + uv * frq;
    
    uv += vec2(sin(phase.x), sin(phase.y)) * amplitude;
}

void doWater(inout vec2 uv)
{
    doWaterFrq(uv, TimeMult1, Frq1, Amplitude1);
    doWaterFrq(uv, TimeMult2, Frq2, Amplitude2);
}
#endif

#ifdef DOF
void doDof(in vec2 uv)
{
    float depth = depthAt(uv);
    float blurRadius = calcDofBlurRadius(depth);
    
    const int dof_blends = 6;
    const int dof_rings = 6;
    
    const float angle_step = PI*2.0 / dof_blends;
    float dist_step = blurRadius / dof_rings;

    vec3 sample_color = darkness_texel(uv, depth);
    float sample_weight_sum = 1;
    for (int i = 1; i <= dof_blends; i += 1)
    {   
        for (int j = 0; j < dof_rings; j += 1)   
        {
            vec2 p;
            p.x = sin(i*angle_step) * dist_step * j;
            p.y = cos(i*angle_step) * dist_step * j;
            vec2 r = dofRand(uv + p) * dist_step / 2;
            vec2 peek = uv + p + r;
            depth = depthAt(peek);
            float localWeight = 1.0 - smoothstep(j + 1, 0, dof_rings);
            sample_color += darkness_texel(peek, depth) * localWeight;
            sample_weight_sum += localWeight;
        }
    }
    sample_color /= sample_weight_sum;
    out_color = vec4(sample_color, 1.0);
}
#endif

void main()
{
    vec2 uv = v_texCoord;

#ifdef LENS_DISTORTION
    doLensDistortion(uv);
#endif

#ifdef WATER
    doWater(uv);
#endif

#ifndef DOF
    out_color = darkness_texel(uv, darkness(depthAt(uv)));
#else
    doDof(uv);
#endif
}
