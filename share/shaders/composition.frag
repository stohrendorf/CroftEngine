layout(bindless_sampler) uniform sampler2D u_texture;
layout(bindless_sampler) uniform sampler2D u_geometryPosition;
layout(bindless_sampler) uniform sampler2D u_portalPosition;
layout(bindless_sampler) uniform sampler2D u_portalPerturb;
#ifdef HBAO
layout(bindless_sampler) uniform sampler2D u_ao;
#endif

#include "flat_pipeline_interface.glsl"
#include "camera_interface.glsl"
#include "time_uniform.glsl"

layout(location=0) out vec4 out_color;

#include "util.glsl"
#include "constants.glsl"

#include "water_deform.glsl"

#if defined(DOF) || defined(FILM_GRAIN)
#include "noise.glsl"
#endif

#ifdef DOF
#include "dof.glsl"
#endif

#ifdef LENS_DISTORTION
#include "lens.glsl"
#endif

void main()
{
    #ifdef IN_WATER
    vec2 uv = (fpi.texCoord - vec2(0.5)) * 0.9 + vec2(0.5);// scale a bit to avoid edge clamping when underwater
    #else
    vec2 uv = fpi.texCoord;
    #endif

    #ifdef FILM_GRAIN
    float grain = noise(uv * time_seconds());
    #endif

    #ifdef LENS_DISTORTION
    do_lens_distortion(uv);
    #endif

    #ifdef IN_WATER
    do_water_distortion(uv);
    #endif

    const vec3 WaterColor = vec3(149.0f / 255.0f, 229.0f / 255.0f, 229.0f / 255.0f);
    #ifdef IN_WATER
    vec3 finalColor = WaterColor;
    #else
    vec3 finalColor = vec3(1.0);
    #endif
    float pDepth = -texture(u_portalPosition, uv).z;
    float geomDepth = -texture(u_geometryPosition, uv).z;
    float whiteness = 0;
    if (geomDepth > pDepth)
    {
        // camera ray goes through water surface; apply perturb
        vec3 dUvSpecular = texture(u_portalPerturb, uv).xyz;
        vec2 pUv = uv + dUvSpecular.xy;
        if (-texture(u_geometryPosition, pUv).z > pDepth) {
            // ...but only apply it if the source pixel's geometry is behind the water surface.
            uv = pUv;
            #ifdef IN_WATER
            finalColor = vec3(1.0);
            #else
            finalColor = WaterColor;
            whiteness = dUvSpecular.z;
            #endif
        }
    }

        #ifndef DOF
    finalColor *= shaded_texel(u_texture, uv, -texture(u_portalPosition, uv).z);
    #else
    finalColor *= do_dof(uv);
    #endif
    finalColor = mix(finalColor, vec3(1), whiteness);

    #ifdef IN_WATER
    float inVolumeRay = pDepth;
    #else
    float inVolumeRay = geomDepth - pDepth;
    #endif
    float d = clamp(inVolumeRay * 2 * InvFarPlane, 0, 1);
    // light absorbtion
    finalColor *= mix(vec3(1), WaterColor, d);
    // light scatter
    finalColor = mix(finalColor, WaterColor, d/30.0);

    #ifdef HBAO
    finalColor *= texture(u_ao, uv).r;
    #endif
    #ifdef FILM_GRAIN
    finalColor *= grain*0.5 + 1.0;
    #endif

    #ifdef VELVIA
    const float VelviaAmount = 0.03;
    const vec2 velviaFac = vec2(2*VelviaAmount + 1.0, -VelviaAmount);
    vec3 velviaColor = vec3(dot(finalColor, velviaFac.xyy), dot(finalColor, velviaFac.yxy), dot(finalColor, velviaFac.yyx));
    finalColor = vec3(1.0) - clamp((-velviaColor*1.01 + vec3(1.0))*1.01, vec3(0.0), vec3(1.0));
    #endif

    out_color = vec4(finalColor, 1.0);
}
