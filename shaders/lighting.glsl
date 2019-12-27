uniform sampler2D u_csmDepth[3];
uniform float u_lightAmbient;

#include "csm_interface.glsl"

struct Light {
    vec3 position;
    float brightness;
    float fadeDistance;
};

layout(std430, binding=2) buffer b_lights {
    Light lights[];
};

float shadow_map_multiplier(in float shadow)
{
    int cascadeIdx = 0;
    while (cascadeIdx < u_csmSplits.length()-1 && -gpi.vertexPos.z > -u_csmSplits[cascadeIdx]) {
        ++cascadeIdx;
    }

    vec3 projCoords = gpi.vertexPosLight[cascadeIdx].xyz / gpi.vertexPosLight[cascadeIdx].w;
    projCoords = projCoords * 0.5 + 0.5;
    float currentDepth = projCoords.z;
    const float bias = 1.0/2048.0;
    float closestDepth = texture(u_csmDepth[cascadeIdx], projCoords.xy).r;
    if (currentDepth + bias >= closestDepth) {
        return shadow;
    }
    else {
        return 1.0;
    }
}

float shadow_map_multiplier() {
    return shadow_map_multiplier(0.5);
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

    return sum;
}

float calc_positional_lighting(in vec3 normal, in vec3 pos)
{
    return calc_positional_lighting(normal, pos, 1);
}
