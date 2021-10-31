#include "csm_interface.glsl"
#include "geometry_pipeline_interface.glsl"

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

const float CSMShadow = 0.2;
float lightNormDot = dot(normalize(gpi.vertexNormalWorld), normalize(csm.lightDir));

float calc_vsm_value(in int splitIdx, in vec3 projCoords)
{
    vec2 moments = vec2(0);
    // https://stackoverflow.com/a/32273875
    #define FETCH_CSM(idx) case idx: moments = texture(u_csmVsm[idx], projCoords.xy).xy; break
    switch (splitIdx) {
        FETCH_CSM(0);
        FETCH_CSM(1);
        FETCH_CSM(2);
        FETCH_CSM(3);
    }
        #undef FETCH_CSM

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

    float result = mix(CSMShadow, 1.0, pMax);
    #ifdef ROOM_SHADOWING
    result = mix(1.0, result, -lightNormDot);
    #endif
    return result;
}

float shadow_map_multiplier()
{
    #ifdef ROOM_SHADOWING
    if (lightNormDot > 0) {
        // ceilings are always fully lit
        return 1.0;
    }
        #endif

    vec3 ps[CSMSplits];
    for (int splitIdx = 0; splitIdx<CSMSplits; ++splitIdx) {
        ps[splitIdx] = gpi.vertexPosLight[splitIdx];
    }
    for (int splitIdx = 0; splitIdx<CSMSplits; ++splitIdx) {
        if (all(greaterThanEqual(ps[splitIdx].xy, vec2(0))) && all(lessThanEqual(ps[splitIdx].xy, vec2(1)))) {
            return calc_vsm_value(splitIdx, ps[splitIdx]);
        }
    }
    return 1.0;
}

vec3 calc_positional_lighting()
{
    if (lights.length() <= 0 || gpi.vertexNormalWorld == vec3(0))
    {
        return vec3(u_lightAmbient);
    }

    vec3 sum = vec3(u_lightAmbient);
    for (int i=0; i<lights.length(); ++i)
    {
        vec3 d = gpi.vertexPosWorld - lights[i].position.xyz;
        float ld = length(d);
        float r = ld / lights[i].fadeDistance;
        float intensity = 1.0 / (r*r + 1.0);
        vec3 color = vec3(lights[i].brightness);
        #if SPRITEMODE == 0
        sum += intensity * clamp(-dot(d/ld, gpi.vertexNormalWorld), 0, 1) * color;
        #else
        sum += intensity * color;
        #endif
    }

    return sum;
}
