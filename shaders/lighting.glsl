layout(binding=1) uniform sampler2D u_csmVsm[5];
layout(location=10) uniform float u_lightAmbient;

#include "csm_interface.glsl"

struct Light {
    vec4 position;
    float brightness;
    float fadeDistance;
    float _pad[2];
};

readonly layout(std430, binding=3) buffer b_lights {
    Light lights[];
};

float shadow_map_multiplier(in vec3 normal, in float shadow)
{
    #ifdef ROOM_SHADOWING
    float d = dot(normalize(normal), normalize(u_csmLightDir));
    if (d > 0) {
        return 1.0;
    }
        #endif

    int splitIdx;
    for (splitIdx=CSMSplits-1; splitIdx>0; --splitIdx) {
        if (-gpi.vertexPos.z > -u_csmSplits[splitIdx]) {
            break;
        }
    }
    vec3 projCoords = gpi.vertexPosLight[splitIdx];
    vec2 moments = texture(u_csmVsm[splitIdx], projCoords.xy).xy;

    float currentDepth = projCoords.z;
    if (currentDepth > 1.0 || currentDepth < moments.x) {
        return 1.0;
    }

    const float ShadowBias = 0.0001;
    float variance = max(-moments.x * moments.x + moments.y, ShadowBias);
    float mD = moments.x - currentDepth;
    float pMax = variance / (mD * mD + variance);

    // light bleeding
    const float BleedBias = 0.05;
    const float BleedBiasRange = 1.0 / (1.0 - BleedBias);
    const float BleedBiasOffset = -BleedBias * BleedBiasRange;
    pMax = clamp(pMax * BleedBiasRange + BleedBiasOffset, 0.0, 1.0);

    float result = mix(shadow, 1.0, pMax);
    #ifdef ROOM_SHADOWING
    result = mix(1.0, result, -d);
    #endif
    return result;
}

float shadow_map_multiplier(in vec3 normal) {
    return shadow_map_multiplier(normal, 0.5);
}

/*
 * @param normal is the surface normal in world space
 * @param pos is the surface position in world space
 */
float calc_positional_lighting(in vec3 normal, in vec3 pos)
{
    if (lights.length() <= 0 || normal == vec3(0))
    {
        return u_lightAmbient;
    }

    normal = normalize(normal);
    float sum = u_lightAmbient;
    for (int i=0; i<lights.length(); ++i)
    {
        vec3 d = pos - lights[i].position.xyz;
        float ld = length(d);
        float r = ld / lights[i].fadeDistance;
        float intensity = lights[i].brightness / (r*r + 1.0);
        sum += intensity * clamp(-dot(d/ld, normal), 0, 1);
    }

    return sum;
}
