#include "csm_interface.glsl"

layout(binding=1) uniform sampler2D u_csmVsm[CSMSplits];
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

float calc_vsm_value(in int splitIdx, in vec3 normal, in float shadow, in float d)
{
    vec3 projCoords = gpi.vertexPosLight[splitIdx];
    vec2 moments;
    // https://stackoverflow.com/a/32273875
    #define FETCH_CSM(idx) case idx: moments = texture(u_csmVsm[idx], projCoords.xy).xy; break
    switch (splitIdx) {
        FETCH_CSM(0);
        FETCH_CSM(1);
        FETCH_CSM(2);
        FETCH_CSM(3);
        FETCH_CSM(4);
    }
        #undef FETCH_CSM

    float currentDepth = projCoords.z;
    const float ShadowSlopeBias = 0.005;
    const float MaxShadowSlopeBias = 2*ShadowSlopeBias;
    float bias = ShadowSlopeBias * tan(acos(clamp(d, 0.0, 1.0)));
    bias = clamp(bias, 0.0, MaxShadowSlopeBias);
    if (currentDepth > 1.0 || currentDepth < moments.x + bias) {
        return 1.0;
    }

    const float ShadowBias = 1e-3;
    float variance = max(-moments.x * moments.x + moments.y, ShadowBias);
    float mD = moments.x - currentDepth;
    float pMax = variance / (mD * mD + variance);

    // light bleeding
    const float BleedBias = 0.005;
    const float BleedBiasRange = 1.0 / (1.0 - BleedBias);
    const float BleedBiasOffset = -BleedBias * BleedBiasRange;
    pMax = clamp(pMax * BleedBiasRange + BleedBiasOffset, 0.0, 1.0);

    float result = mix(shadow, 1.0, pMax);
    #ifdef ROOM_SHADOWING
    result = mix(1.0, result, -d);
    #endif
    return result;
}

float shadow_map_multiplier(in vec3 normal, in float shadow)
{
    float d = dot(normalize(normal), normalize(u_csmLightDir));
    #ifdef ROOM_SHADOWING
    if (d > 0) {
        return 1.0;
    }
        #endif

    int splitIdx;
    for (splitIdx = 0; splitIdx<CSMSplits; ++splitIdx) {
        vec2 p = gpi.vertexPosLight[splitIdx].xy;
        if (all(greaterThanEqual(p, vec2(0))) && all(lessThanEqual(p, vec2(1)))) {
            break;
        }
    }
    if (splitIdx == CSMSplits) return 1.0;
    return calc_vsm_value(splitIdx, normal, shadow, d);
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
