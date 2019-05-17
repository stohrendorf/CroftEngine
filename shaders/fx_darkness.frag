uniform sampler2D u_texture;
uniform mat4 u_projection;
uniform float u_time;

in vec2 v_texCoord;

out vec4 out_color;

const float PI = 3.14159265;
const float Z_max = 20480;

#include "util.glsl"

#ifdef WATER
#include "water.glsl"
#endif

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

void main()
{
    vec2 uv = v_texCoord;

    float grain = rand1(uv);

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

    out_color.rgb *= grain*0.3 + 0.7;
    out_color.a = 1;
}
