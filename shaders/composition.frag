uniform sampler2D u_texture;
uniform sampler2D u_portalDepth;
uniform sampler2D u_portalPerturb;
uniform sampler2D u_ao;
layout(location=6) uniform float u_time;

#include "flat_pipeline_interface.glsl"
#include "camera_interface.glsl"

layout(location=0) out vec4 out_color;

#include "util.glsl"
#include "constants.glsl"

#include "water.glsl"

#define DOF

#include "depth.glsl"
#ifdef DOF
#include "dof.glsl"
#endif

#ifdef LENS_DISTORTION
#include "lens.glsl"
#endif

float rand1(in vec2 seed)
{
    const vec2 K1 = vec2(
    23.14069263277926, // e^pi (Gelfond's constant)
    2.665144142690225// 2^sqrt(2) (Gelfond–Schneider constant)
    );
    return fract(cos(dot(seed*u_time/40, K1)) * 12345.6789);
}

float fbm(in vec2 uv) {
    // Initial values
    float value = 0.0;
    float amplitude = .5;
    float frequency = 0.;
    //
    // Loop of octaves
    const int octaves = 4;
    for (int i = 0; i < octaves; i++) {
        value += amplitude * rand1(uv);
        uv *= 2.;
        amplitude *= .5;
    }
    return value;
}

void main()
{
    #ifdef WATER
    vec2 uv = (fpi.texCoord - vec2(0.5)) * 0.9 + vec2(0.5);// scale a bit to avoid edge clamping when underwater
    #else
    vec2 uv = fpi.texCoord;
    #endif

    float grain = rand1(uv);

    #ifdef LENS_DISTORTION
    do_lens_distortion(uv);
    #endif

    #ifdef WATER
    do_water_distortion(uv);
    #endif

    float d = depth_at(uv) - depth_at(u_portalDepth, uv);
    d = clamp(d*4, 0, 1);
    if (d > 0)
    {
        // camera ray goes through water surface; apply perturb
        uv += texture(u_portalPerturb, uv).xy * 512;
    }

    vec3 finalColor;

    #ifndef DOF
    finalColor = shaded_texel(u_texture, uv, depth_at(u_portalDepth, uv));
    #else
    finalColor = do_dof(uv);
    #endif

    const vec3 WaterColor = vec3(149.0f / 255.0f, 229.0f / 255.0f, 229.0f / 255.0f);
    #ifdef WATER
    d = clamp(depth_at(u_portalDepth, uv)*4, 0, 1);
    // light absorbtion
    finalColor *= mix(vec3(1), WaterColor, d);
    // light scatter
    finalColor = mix(finalColor, WaterColor, d/30);
    #else
    if (d > 0)
    {
        // light absorbtion
        finalColor *= mix(vec3(1), WaterColor, d);
        // light scatter
        finalColor = mix(finalColor, WaterColor, d/30);
    }
        #endif

    finalColor *= texture(u_ao, uv).r*0.8 + 0.2;
    finalColor *= grain*0.3 + 0.7;

    const float velviaAmount = 0.1;

    const vec2 velviaFac = vec2(1.0 + 2*velviaAmount, -velviaAmount);

    vec3 velviaColor = vec3(dot(finalColor, velviaFac.xyy), dot(finalColor, velviaFac.yxy), dot(finalColor, velviaFac.yyx));
    finalColor = vec3(1.0) - clamp((vec3(1.0) - velviaColor*1.01)*1.01, vec3(0.0), vec3(1.0));

    out_color.rgb = finalColor;
    out_color.a = 1;
}
