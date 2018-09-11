uniform vec3 u_lightPosition;
uniform float u_baseLight;
uniform float u_baseLightDiff;

/*
 * @param normal is the surface normal in world space
 * @param pos is the surface position in world space
 */
float calcShadeFactor(in vec3 normal, in vec3 pos)
{
    if(isnan(u_lightPosition.x) || normal == vec3(0))
    {
        return clamp(u_baseLight, 0, 1);
    }
    else
    {
        // phong without the specular component
        const float k_d = 0.8;
        float Ka_Ia = u_baseLight;
        float Ka = u_baseLightDiff;

        vec3 Lm = normalize(u_lightPosition - pos);
        return clamp(Ka_Ia + k_d * dot(normal, Lm) * Ka, 0, 1);
    }
}
