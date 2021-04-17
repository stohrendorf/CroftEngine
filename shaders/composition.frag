uniform sampler2D u_texture;
uniform sampler2D u_linearPortalDepth;
uniform sampler2D u_portalPerturb;
uniform sampler2D u_ao;
uniform sampler2D u_linearDepth;

#include "flat_pipeline_interface.glsl"
#include "camera_interface.glsl"
#include "time_uniform.glsl"

layout(location=0) out vec4 out_color;

#include "util.glsl"
#include "constants.glsl"

#include "water_deform.glsl"

#ifdef DOF
#include "dof.glsl"
#endif

#ifdef LENS_DISTORTION
#include "lens.glsl"
#endif

void main()
{
#ifdef WATER
    vec2 uv = (fpi.texCoord - vec2(0.5)) * 0.9 + vec2(0.5);// scale a bit to avoid edge clamping when underwater
#else
    vec2 uv = fpi.texCoord;
    #endif

    #ifdef FILM_GRAIN
    float grain = rand1(uv);
    #endif

    #ifdef LENS_DISTORTION
    do_lens_distortion(uv);
    #endif

    #ifdef WATER
    do_water_distortion(uv);
    #endif

    const vec3 WaterColor = vec3(149.0f / 255.0f, 229.0f / 255.0f, 229.0f / 255.0f);
    #ifdef WATER
    vec3 finalColor = WaterColor;
    #else
    vec3 finalColor = vec3(1.0);
    #endif
    float pDepth = texture(u_linearPortalDepth, uv).r;
    float geomDepth = texture(u_linearDepth, uv).r;
    float d = geomDepth - pDepth;
    d = clamp(d*4, 0, 1);
    if (d > 0)
    {
        // camera ray goes through water surface; apply perturb
        vec2 pUv = uv + texture(u_portalPerturb, uv).xy * 64;
        if (texture(u_linearDepth, pUv).r >= pDepth) {
            // ...but only apply it if the source pixel's geometry is below the water surface.
            uv = pUv;
            #ifdef WATER
            finalColor = vec3(1.0);
            #else
            finalColor = WaterColor;
            #endif
        }
    }

        #ifndef DOF
    finalColor *= shaded_texel(u_texture, uv, texture(u_linearPortalDepth, uv).r);
    #else
    finalColor *= do_dof(uv);
    #endif

    #ifdef WATER
    d = clamp(pDepth*4, 0, 1);
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

    finalColor *= texture(u_ao, uv).r;
    #ifdef FILM_GRAIN
    finalColor *= grain*0.3 + 0.7;
    #endif

    const float velviaAmount = 0.03;

    const vec2 velviaFac = vec2(1.0 + 2*velviaAmount, -velviaAmount);

    vec3 velviaColor = vec3(dot(finalColor, velviaFac.xyy), dot(finalColor, velviaFac.yxy), dot(finalColor, velviaFac.yyx));
    finalColor = vec3(1.0) - clamp((vec3(1.0) - velviaColor*1.01)*1.01, vec3(0.0), vec3(1.0));

    out_color.rgb = finalColor;
    out_color.a = 1;
}
