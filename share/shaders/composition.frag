layout(bindless_sampler) uniform sampler2D u_texture;
layout(bindless_sampler) uniform sampler2D u_geometryPosition;
layout(bindless_sampler) uniform sampler2D u_portalPosition;
layout(bindless_sampler) uniform sampler2D u_portalPerturb;

#include "flat_pipeline_interface.glsl"
#include "camera_interface.glsl"
#include "time_uniform.glsl"

layout(location=2) uniform vec3 u_waterColor;
layout(location=3) uniform float u_waterDensity;

layout(location=0) out vec3 out_color;

#include "util.glsl"
#include "constants.glsl"

#ifdef DOF
#include "noise.glsl"
#include "dof.glsl"
#endif

void main()
{
    const float WaterSurfaceMultiplier = 0.6;

    #ifdef IN_WATER
    vec3 finalColor = vec3(WaterSurfaceMultiplier);
    #else
    vec3 finalColor = vec3(1.0);
    #endif

    vec2 uv = fpi.texCoord;
    float pDepth = -texture(u_portalPosition, uv).x;
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
        finalColor = vec3(WaterSurfaceMultiplier);
        whiteness = dUvSpecular.z;
        #endif
    }

        #ifndef DOF
    finalColor *= texture(u_texture, uv).rgb * texel_shade(shadeDepth);
    #else
    finalColor *= do_dof(uv);
    #endif
    finalColor = mix(finalColor, vec3(1) * texel_shade(shadeDepth), whiteness);

    #ifdef IN_WATER
    float inVolumeRay = min(geomDepth, pDepth);
    #else
    float inVolumeRay = geomDepth - pDepth;
    #endif
    float inVolumeRayNorm = clamp(inVolumeRay * InvFarPlane, 0, 1);

    finalColor = mix(u_waterColor, finalColor, exp(-inVolumeRayNorm * u_waterDensity));

    out_color = finalColor;
}
