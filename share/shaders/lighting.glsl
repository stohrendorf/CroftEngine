#include "csm_interface.glsl"
#include "geometry_pipeline_interface.glsl"
#include "util.glsl"

layout(bindless_sampler) uniform sampler2D u_csmVsm[CSMSplits];
layout(location=10) uniform float u_lightAmbient;

struct Light {
    vec4 position;
    vec4 color;
    float fadeDistance;
    float _pad[3];
};

layout(std430, binding=3) readonly restrict buffer b_lights {
    Light lights[];
};

layout(std430, binding=4) readonly restrict buffer b_dynLights {
    Light dynLights[];
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
        return moments.x;
    }

    const float ShadowBias = 0.005;
    float variance = max(-moments.x * moments.x + moments.y, ShadowBias);
    float pMax = variance / (mD * mD + variance);

    pMax = clamp(pMax, 0.0, 1.0);

    float result = mix(CSMShadow, 1.0, pMax);
    #ifdef ROOM_SHADOWING
    result = mix(1.0, result, -lightNormDot);
    #endif
    return result*0.000001 + moments.x;
}

float shadow_map_multiplier()
{
    #ifdef ROOM_SHADOWING
    if (u_lightingMode == 0 && lightNormDot > 0) {
        // ceilings are always fully lit
        return 1.0;
    }
    #endif

    for (int splitIdx = 0; splitIdx<CSMSplits; ++splitIdx) {
        vec3 p = gpi.vertexPosLight[splitIdx];
        if (all(greaterThanEqual(p.xy, vec2(0))) && all(lessThanEqual(p.xy, vec2(1)))) {
            return calc_vsm_value(splitIdx, p);
        }
    }
    return 1.0;
}

float calc_light_strength(in vec3 pos, in float fadeDistance)
{
    vec3 d = gpi.vertexPosWorld - pos;
    float ld = length(d);
    float r = ld / fadeDistance;
    float result = 1.0 / (r*r + 1.0);
    #if SPRITEMODE == 0
    return result * clamp(-dot(d/ld, gpi.vertexNormalWorld), 0.0, 1.0);
    #else
    return result;
    #endif
}

vec3 calc_positional_lighting()
{
    vec3 sum;
    switch (u_lightingMode) {
        case 0:
        sum = vec3(u_lightAmbient);
        break;
        case 1:
        sum = vec3(u_lightAmbient) * 0.5;
        break;
        case 2:
        sum = vec3(u_lightAmbient) * 0.1;
        break;
        default :
        sum = vec3(u_lightAmbient) * 0.03;
        break;
    }

    if (gpi.vertexNormalWorld == vec3(0))
    {
        return sum;
    }

    for (int i=0; i<lights.length(); ++i)
    {
        sum += vec3(calc_light_strength(lights[i].position.xyz, lights[i].fadeDistance)) * toLinear(lights[i].color.xyz);
    }

    for (int i=0; i<dynLights.length(); ++i)
    {
        sum += vec3(calc_light_strength(dynLights[i].position.xyz, dynLights[i].fadeDistance)) * toLinear(dynLights[i].color.xyz);
    }

    return sum;
}
