uniform sampler2D u_texture;
uniform sampler2D u_portalDepth;
uniform sampler2D u_ao;
uniform mat4 u_camProjection;
uniform float u_time;

in vec2 v_texCoord;

layout(location=0) out vec4 out_color;

const float PI = 3.14159265;
const float Z_max = 20480;

#include "util.glsl"

#include "water.glsl"

#define DOF

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
        2.665144142690225 // 2^sqrt(2) (Gelfond–Schneider constant)
    );
    return fract( cos( dot(seed*u_time/40, K1) ) * 12345.6789 );
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
    vec2 uv = v_texCoord;

    float grain = rand1(uv);

#ifdef LENS_DISTORTION
    do_lens_distortion(uv);
#endif

#ifdef WATER
    do_water_distortion(uv);
#else
    float d = depth_at(uv) - depth_at(u_portalDepth, uv);
    d = clamp(d*4, 0, 1);
    if( d > 0 )
    {
        // camera ray goes through water surface; scale distortion with underwater ray length
        do_water_distortion_frq(uv, 0.005, 22.6, 0.00175*fbm(uv));
    }
#endif

#ifndef DOF
    out_color.rgb = shaded_texel(uv, depth_at(uv));
#else
    out_color.rgb = do_dof(uv);
#endif

    const vec4 WaterColor = vec4(149.0f / 255.0f, 229.0f / 255.0f, 229.0f / 255.0f, 1);
#ifdef WATER
    float d = clamp(depth_at(u_portalDepth, uv)*4, 0, 1);
    // light absorbtion
    out_color *= mix(vec4(1), WaterColor, d);
    // light scatter
    out_color = mix(out_color, WaterColor, d/30);
#else
    if( d > 0 )
    {
        // light absorbtion
        out_color *= mix(vec4(1), WaterColor, d);
        // light scatter
        out_color = mix(out_color, WaterColor, d/30);
    }
#endif

    out_color.rgb *= texture(u_ao, uv).r;
    out_color.rgb *= grain*0.3 + 0.7;
    out_color.a = 1;
}
