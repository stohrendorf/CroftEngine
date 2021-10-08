#include "csm_interface.glsl"

layout(bindless_sampler) uniform sampler2D u_csmVsm[CSMSplits];
layout(location=10) uniform float u_lightAmbient;

struct Light {
    vec4 position;
    float brightness;
    float fadeDistance;
    float _pad[2];
};

layout(std430, binding=3) readonly restrict buffer b_lights {
    Light lights[];
};

float calc_vsm_value(in int splitIdx, in float shadow, in float lightNormDot, in vec3 projCoords)
{
    vec2 moments = texture(u_csmVsm[splitIdx], projCoords.xy).xy;

    float currentDepth = clamp(projCoords.z, 0.0, 1.0);

    const float ShadowSlopeBias = 0.005;
    const float MaxShadowSlopeBias = 2*ShadowSlopeBias;
    //float bias = ShadowSlopeBias * sqrt(1.0 - lightNormDot*lightNormDot);
    float bias = max(MaxShadowSlopeBias * (1.0 - lightNormDot), ShadowSlopeBias);
    bias = clamp(bias, 0.0, MaxShadowSlopeBias);
    float mD = moments.x - currentDepth;
    if (mD + bias > 0) {
        return 1.0;
    }

    const float ShadowBias = 0.005;
    float variance = max(-moments.x * moments.x + moments.y, ShadowBias);
    float pMax = variance / (mD * mD + variance);

    pMax = clamp(pMax, 0.0, 1.0);

    float result = mix(shadow, 1.0, pMax);
    #ifdef ROOM_SHADOWING
    result = mix(1.0, result, -lightNormDot);
    #endif
    return result;
}

float shadow_map_multiplier(in vec3 worldNormal, in float shadow)
{
    float lightNormDot = dot(normalize(worldNormal), normalize(csm.lightDir));
    #ifdef ROOM_SHADOWING
    if (lightNormDot > 0) {
        // ceilings are always fully lit
        return 1.0;
    }
        #endif

    for (int splitIdx = 0; splitIdx<CSMSplits; ++splitIdx) {
        vec3 p = gpi.vertexPosLight[splitIdx];
        if (p.x >= 0.0 && p.y >= 0 && p.x <= 1 && p.y <= 1) {
            return calc_vsm_value(splitIdx, shadow, lightNormDot, p);
        }
    }
    return 1.0;
}

float shadow_map_multiplier(in vec3 worldNormal) {
    return shadow_map_multiplier(worldNormal, 0.2);
}

float calc_positional_lighting(in vec3 worldNormal, in vec3 worldPos)
{
    if (lights.length() <= 0 || worldNormal == vec3(0))
    {
        return u_lightAmbient;
    }

        #if SPRITEMODE == 0
    worldNormal = normalize(worldNormal);
    #endif
    float sum = u_lightAmbient;
    for (int i=0; i<lights.length(); ++i)
    {
        vec3 d = worldPos - lights[i].position.xyz;
        float ld = length(d);
        float r = ld / lights[i].fadeDistance;
        float intensity = lights[i].brightness / (r*r + 1.0);
        #if SPRITEMODE == 0
        sum += intensity * clamp(-dot(d/ld, worldNormal), 0, 1);
        #else
        sum += intensity;
        #endif
    }

    return sum;
}
