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

#ifdef DOF
#include "noise.glsl"
#include "dof.glsl"
#endif

void main()
{
    const vec3 WaterColor = vec3(149.0 / 255.0, 229.0 / 255.0, 229.0 / 255.0);

    #ifdef IN_WATER
    vec3 finalColor = WaterColor;
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
        finalColor = WaterColor;
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
    float d = clamp(inVolumeRay * InvFarPlane, 0, 1);
    // light absorbtion
    finalColor *= mix(vec3(1), WaterColor*0.5, d);
    // light scatter
    finalColor = mix(finalColor, shade_texel(WaterColor*0.5, shadeDepth), d*d);

    out_color = vec4(finalColor, 1.0);
}
