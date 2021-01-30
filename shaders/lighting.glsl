layout(binding=1) uniform sampler2D u_csmVsm1;
layout(binding=1) uniform sampler2D u_csmVsm2;
layout(binding=1) uniform sampler2D u_csmVsm3;
layout(location=10) uniform float u_lightAmbient;

#include "csm_interface.glsl"

struct Light {
    vec3 position;
    float brightness;
    float fadeDistance;
    float _pad[2];
};

readonly layout(std430, binding=3) buffer b_lights {
    Light lights[];
};

float shadow_map_multiplier(in vec3 normal, in float shadow)
{
    vec3 projCoords;
    vec2 moments;

    if (-gpi.vertexPos.z > -u_csmSplits3) {
        projCoords = gpi.vertexPosLight3;
        moments = texture(u_csmVsm3, projCoords.xy).xy;
    }
    else if (-gpi.vertexPos.z > -u_csmSplits2) {
        projCoords = gpi.vertexPosLight2;
        moments = texture(u_csmVsm2, projCoords.xy).xy;
    }
    else {
        projCoords = gpi.vertexPosLight1;
        moments = texture(u_csmVsm1, projCoords.xy).xy;
    }

    float currentDepth = projCoords.z;
    if (currentDepth > 1.0 || currentDepth < moments.x) {
        return 1.0;
    }

    const float ShadowBias = 0.0001;
    float variance = max(moments.y - moments.x * moments.x, ShadowBias);
    float mD = moments.x - currentDepth;
    float pMax = variance / (variance + mD * mD);

    // light bleeding
    const float BleedBias = 0.05;
    pMax = clamp((pMax - BleedBias) / (1.0 - BleedBias), 0.0, 1.0);

    return mix(shadow, 1.0, pMax);
}

float shadow_map_multiplier(in vec3 normal) {
    return shadow_map_multiplier(normal, 0.5);
}

/*
 * @param normal is the surface normal in world space
 * @param pos is the surface position in world space
 */
float calc_positional_lighting(in vec3 normal, in vec3 pos, in float n)
{
    if (lights.length() <= 0 || normal == vec3(0))
    {
        return u_lightAmbient;
    }

    normal = normalize(normal);
    float sum = u_lightAmbient;
    for (int i=0; i<lights.length(); ++i)
    {
        vec3 d = pos - lights[i].position;
        float intensity = lights[i].brightness / (1 + length(d)/lights[i].fadeDistance);
        vec3 light_dir = normalize(d);
        sum += pow(intensity * max(dot(light_dir, normal), 0), n);
    }

    return pow(sum, 0.75);
}

float calc_positional_lighting(in vec3 normal, in vec3 pos)
{
    return calc_positional_lighting(normal, pos, 1.5);
}
