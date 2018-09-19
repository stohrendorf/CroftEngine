uniform sampler2D u_texture;
uniform mat4 u_projection;

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
