layout(binding=1) uniform sampler2D u_csmVsm[3];
layout(location=10) uniform float u_lightAmbient;

#include "csm_interface.glsl"

struct Light {
    vec3 position;
    float brightness;
    float fadeDistance;
};

readonly layout(std430, binding=3) buffer b_lights {
    Light lights[];
};

float shadow_map_multiplier(in vec3 normal, in float shadow)
{
    int cascadeIdx = 0;
    while (cascadeIdx < u_csmSplits.length()-1 && -gpi.vertexPos.z > -u_csmSplits[cascadeIdx]) {
        ++cascadeIdx;
    }

    vec3 projCoords = gpi.vertexPosLight[cascadeIdx].xyz / gpi.vertexPosLight[cascadeIdx].w;
    projCoords = projCoords * 0.5 + 0.5;
    float currentDepth = projCoords.z;
    if (currentDepth > 1.0) {
        return 1.0;
    }

    float cosTheta = abs(dot(normalize(normal), normalize(u_csmLightDir)));
    const float BiasExceedOne = sqrt(0.5);
    float bias = cosTheta > BiasExceedOne ? sqrt(1-cosTheta*cosTheta) / cosTheta : 1;
    currentDepth -= 0.01*bias;

    vec2 moments = texture(u_csmVsm[cascadeIdx], projCoords.xy).xy;
    if (currentDepth < moments.x) {
        return 1.0;
    }

    float variance = moments.y - moments.x * moments.x;
    float mD = currentDepth - moments.x;
    float p = variance / (variance + mD * mD);
    return mix(shadow, 1.0, clamp(p, 0, 1));
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
    float sum = 0;
    float brightest = u_lightAmbient;// the brightest light gives ambient brightness if it is brighter than ambient
    for (int i=0; i<lights.length(); ++i)
    {
        vec3 d = pos - lights[i].position;
        float intensity = lights[i].brightness / (1 + length(d)/lights[i].fadeDistance);
        vec3 light_dir = normalize(d);
        sum += pow(intensity * max(dot(light_dir, normal), 0), n);

        brightest = max(brightest, intensity);
    }

    return sum + (brightest + u_lightAmbient)/2;
}

float calc_positional_lighting(in vec3 normal, in vec3 pos)
{
    return calc_positional_lighting(normal, pos, 1);
}
