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

    float sum = u_lightAmbient;
    for (int i=0; i<u_numLights; ++i)
    {
        vec3 light_dir = normalize(pos - u_lights[i].position);
        sum += u_lights[i].intensity * max(dot(light_dir, normalize(normal)), 0);
    }

    return sum;
}
