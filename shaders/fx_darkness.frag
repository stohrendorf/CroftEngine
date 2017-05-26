uniform sampler2D u_texture;
uniform sampler2D u_depth;
uniform mat4 u_projection;

varying vec2 v_texCoord;

out vec4 out_color;

#define PI 3.14159265

#define DOF

#ifdef DOF
#ifdef WATER
const float dof_sensor_size = 80.0 / 20480;
const float dof_focal = 512.0 / 20480;
#else
const float dof_sensor_size = 20.0 / 20480;
const float dof_focal = 1536.0 / 20480;
#endif

const float dof_threshold = 0.8;
const float dof_gain = 3.0;

// get an over-exposed pixel
vec3 dofColor(in vec2 uv)
{
    vec3 col = texture2D(u_texture, uv).rgb;
    
    const vec3 luminance_factor = vec3(0.299, 0.587, 0.114);
    float luminance = dot(col, luminance_factor);
    float exposure = 1 + max((luminance-dof_threshold)*dof_gain, 0.0);
    return col * exposure;
}

// get a random 2D-vector, each component within -1..1
vec2 dofRand(vec2 uv)
{
    float noiseX = clamp(fract(sin(dot(uv, vec2(12.9898,78.233)    )) * 43758.5453), 0.0, 1.0)*2.0-1.0;
    float noiseY = clamp(fract(sin(dot(uv, vec2(12.9898,78.233)*2.0)) * 43758.5453), 0.0, 1.0)*2.0-1.0;
    return vec2(noiseX, noiseY);
}

float calcDofBlurRadius(in float depth) {
    float d = abs(depth - dof_focal) / dof_focal;
    return d * dof_sensor_size;
}
#endif

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
    return clamp(d, 0, 1);
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

    float depth;
#ifdef WATER
    depth = depthAt(uv);

    float time = u_time * TimeMult1;
    vec2 angle = time + uv * Frq1;
    
    uv += vec2(sin(angle.x), sin(angle.y)) * Amplitude1 * (depth + 0.1);

    time = u_time * TimeMult2;
    angle = time + uv * Frq2;

    uv += vec2(sin(angle.x), sin(angle.y)) * Amplitude2 * (depth + 0.1);
#endif

    depth = depthAt(uv);
    out_color = texture(u_texture, uv) * (1-depth);

#ifdef DOF
    float blurRadius = calcDofBlurRadius(depth);
    
    const int dof_blends = 6;
    const int dof_rings = 6;
    
    const float angle_step = PI*2.0 / dof_blends;
    float dist_step = blurRadius / dof_rings;

    float peek_sum = 1;
    for (int i = 1; i <= dof_blends; i += 1)
    {   
        for (int j = 0; j < dof_rings; j += 1)   
        {
            vec2 p;
            p.x = sin(i*angle_step) * dist_step * j;
            p.y = cos(i*angle_step) * dist_step * j;
            vec2 r = dofRand(uv + p) * dist_step / 2;
            vec2 peek = uv + p + r;
            float peek_d = depthAt(peek);
            float peek_f = abs(dof_focal - peek_d) / dof_focal;
            out_color.rgb += dofColor(peek) * peek_f * (1-peek_d);
            peek_sum += peek_f;
        }
    }
    out_color.rgb /= peek_sum; //divide by sample count
#endif
}
