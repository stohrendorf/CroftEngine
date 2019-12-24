uniform float u_lightAmbient;
uniform sampler2D u_lightDepth[3];
uniform float u_csmSplits[3];

in vec4 v_vertexPosLight[3];

struct Light {
    vec3 position;
    float brightness;
    float fadeDistance;
};

layout(std430) buffer b_lights {
    Light lights[];
};

float shadow_map_multiplier()
{
    int cascadeIdx = 0;
    while (cascadeIdx < u_csmSplits.length()-1 && -v_vertexPos.z > -u_csmSplits[cascadeIdx]) {
        ++cascadeIdx;
    }

    vec3 projCoords = v_vertexPosLight[cascadeIdx].xyz / v_vertexPosLight[cascadeIdx].w;
    projCoords = projCoords * 0.5 + 0.5;
    float currentDepth = projCoords.z;
    const float bias = 0.005;
    const float d = 1.0/2048.0;
    int n = 0;
    for (int x=-1; x<=1; ++x) {
        for (int y=-1; y<=1; ++y) {
            float closestDepth = texture(u_lightDepth[cascadeIdx], projCoords.xy + vec2(x*d, y*d)).r;
            if (currentDepth + bias >= closestDepth) {
                ++n;
            }
        }
    }
    return mix(1.0, 0.5, smoothstep(0.0, 9.0, n));
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
