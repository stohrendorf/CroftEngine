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

    float cosTheta = clamp(dot(normalize(normal), normalize(u_csmLightDir)), 0, 1);
    float bias = clamp(0.01*tan(acos(cosTheta)), 0, 0.01);
    currentDepth -= bias;

    vec2 moments = texture(u_csmVsm[cascadeIdx], projCoords.xy).xy;
    if (currentDepth <= moments.x) {
        return 1.0;
    }

    float variance = moments.y - moments.x * moments.x;
    float mD = currentDepth - moments.x;
    float mD_2 = mD * mD;
    float p = variance / (variance + mD_2);
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

    // PBR stuff
    #include "constants.glsl"

float DistributionGGX(in vec3 N, in vec3 H, in float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(in float NdotV, in float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(in vec3 N, in vec3 V, in vec3 L, in float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(in float cosTheta, in vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 calc_positional_pbr_lighting(in vec3 camPos, in vec3 normal, in vec3 pos, in vec3 albedo)
{
    if (lights.length() <= 0 || normal == vec3(0))
    {
        return u_lightAmbient * albedo;
    }

    vec3 N = normalize(normal);
    vec3 V = normalize(camPos - pos);

    const vec3 F0 = vec3(0.04);
    const vec3 lightColor = vec3(0.8, 0.9, 1.0);
    const float roughness = 0.8;

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < lights.length(); ++i)
    {
        // calculate per-light radiance
        vec3 L = normalize(lights[i].position - pos);
        vec3 H = normalize(V + L);
        vec3 d = pos - lights[i].position;
        float intensity = lights[i].brightness / (1 + length(d)/lights[i].fadeDistance);
        vec3 radiance     = lightColor * intensity / 2;

        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular     = numerator / max(denominator, 0.001);

        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo * u_lightAmbient;
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    return color;
}
