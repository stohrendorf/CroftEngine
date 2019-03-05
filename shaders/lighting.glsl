uniform vec3 u_lightPosition;
uniform float u_baseLight;
uniform float u_baseLightDiff;

/*
 * @param normal is the surface normal in world space
 * @param pos is the surface position in world space
 */
float calc_positional_lighting(in vec3 normal, in vec3 pos, in vec3 camspace_pos)
{
    if(isnan(u_lightPosition.x) || normal == vec3(0))
    {
        return clamp(u_baseLight, 0, 1);
    }

    vec3 light_dir = normalize(pos - u_lightPosition);
    float value = dot(light_dir, normal);

    if(value <= 0)
        return clamp(u_baseLight/2, 0, 1);

    // Incident vector is opposite light direction vector.
    float eDotR = dot(normalize(camspace_pos), reflect(-light_dir, normal));

    if (eDotR > 0.0)
    {
        const float SpecExp = 50;
        value += pow(eDotR, SpecExp);
    }

    return clamp(u_baseLight/2 + u_baseLightDiff*2 * value, 0, 1);
}
