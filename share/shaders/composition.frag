layout(bindless_sampler) uniform sampler2D u_texture;
layout(bindless_sampler) uniform sampler2D u_geometryPosition;
layout(bindless_sampler) uniform sampler2D u_portalPosition;
layout(bindless_sampler) uniform sampler2D u_portalPerturb;

#include "flat_pipeline_interface.glsl"
#include "camera_interface.glsl"
#include "time_uniform.glsl"

layout(location=0) out vec4 out_color;

#include "util.glsl"
#include "constants.glsl"

#ifdef IN_WATER
#include "water_deform.glsl"
#endif

#ifdef DOF
#include "noise.glsl"
#include "dof.glsl"
#endif

void main()
{
    const vec3 WaterColor = vec3(149.0f / 255.0f, 229.0f / 255.0f, 229.0f / 255.0f);

    #ifdef IN_WATER
    vec2 uv = (fpi.texCoord - vec2(0.5)) * 0.9 + vec2(0.5);// scale a bit to avoid edge clamping when underwater
    do_water_distortion(uv);
    vec3 finalColor = WaterColor;
    #else
    vec2 uv = fpi.texCoord;
    vec3 finalColor = vec3(1.0);
    #endif

    float pDepth = -texture(u_portalPosition, uv).z;
    float geomDepth = -texture(u_geometryPosition, uv).z;
    vec3 dUvSpecular = texture(u_portalPerturb, uv).xyz;
    vec2 pUv = uv + dUvSpecular.xy;
    float pUvD = -texture(u_geometryPosition, pUv).z;
    float whiteness = 0;
    float shadeDepth = geomDepth;
    if (min(geomDepth, pUvD) > pDepth)
    {
        // camera ray goes through water surface; apply perturb
        // ...but only apply it if the source pixel's geometry is behind the water surface.
        uv = pUv;
        shadeDepth = pUvD;
        #ifdef IN_WATER
        finalColor = vec3(1.0);
        #else
        finalColor = WaterColor;
        whiteness = dUvSpecular.z;
        #endif
    }

        #ifndef DOF
    finalColor *= texture(u_texture, uv).rgb;
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

    finalColor = shade_texel(finalColor, shadeDepth);
    out_color = vec4(finalColor, 1.0);
}
