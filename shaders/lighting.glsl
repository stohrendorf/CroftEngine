uniform vec3 u_lightPosition;
uniform float u_lightAmbient;
uniform float u_lightIntensity;

/*
 * @param normal is the surface normal in world space
 * @param pos is the surface position in world space
 */
float calc_positional_lighting(in vec3 normal, in vec3 pos)
{

    if(isnan(u_lightPosition.x) || normal == vec3(0))
    {
        return u_lightAmbient;
    }

    vec3 light_dir = normalize(pos - u_lightPosition);
    float value = max(dot(light_dir, normalize(normal)), 0);

    if(value <= 0)
        return u_lightAmbient;

    return u_lightAmbient + max( u_lightIntensity-u_lightAmbient, 0.0 ) * value;
}
