uniform vec3 u_lightPosition;
uniform float u_baseLight;
uniform float u_baseLightDiff;

float calcShadeFactor(in vec3 normal, in vec3 pos)
{
    float shadeFactor;
    if(isnan(u_lightPosition.x) || normal == vec3(0))
    {
        shadeFactor = clamp(u_baseLight, 0, 1);
    }
    else
    {
        // diffuse
        vec3 dir = normalize(vec4(u_lightPosition, 1).xyz - pos);
        shadeFactor = clamp(u_baseLight + dot(normal, dir) * u_baseLightDiff, 0, 1);

        // specular
        const float specularStrength = 0.5;
        const float specularPower = 4;
        vec3 viewDir = normalize(-pos);
        vec3 reflectDir = reflect(-dir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), specularPower);
        shadeFactor += specularStrength * spec;
    }
    return shadeFactor;
}
