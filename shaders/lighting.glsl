uniform float u_lightAmbient;

struct Light {
    vec3 position;
    float intensity;
};

uniform Light u_lights[8];
uniform int u_numLights;

/*
 * @param normal is the surface normal in world space
 * @param pos is the surface position in world space
 */
float calc_positional_lighting(in vec3 normal, in vec3 pos)
{
    if (u_numLights <= 0 || normal == vec3(0))
    {
        return u_lightAmbient;
    }

    normal = normalize(normal);
    float sum = u_lightAmbient;
    for (int i=0; i<u_numLights; ++i)
    {
        vec3 light_dir = normalize(pos - u_lights[i].position);
        sum += u_lights[i].intensity * max(dot(light_dir, normal), 0);
    }

    return sum;
}

float calc_positional_lighting(in vec3 normal, in vec3 pos, in float k_diffuse, in float k_specular, in float n)
{
    if (u_numLights <= 0 || normal == vec3(0))
    {
        return u_lightAmbient;
    }

    normal = normalize(normal);
    float sum = u_lightAmbient;
    for (int i=0; i<u_numLights; ++i)
    {
        vec3 light_dir = normalize(pos - u_lights[i].position);
        float dotLN = dot(light_dir, normal);
        if(dotLN <= 0)
            continue;

        vec3 R = reflect(normal, light_dir);
        float Iin = u_lights[i].intensity;
        sum += Iin * (k_diffuse*dotLN + k_specular*(n+2)/(6.3)*pow(dot(normal, R), n));
    }

    return sum;
}
