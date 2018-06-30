uniform vec3 u_lightPosition;
uniform float u_baseLight;
uniform float u_baseLightDiff;

float calcShadeFactor(in vec3 normal, in vec3 pos)
{
    float specular;
    if(isnan(u_lightPosition.x) || normal == vec3(0))
    {
        specular = clamp(u_baseLight, 0, 1);
    }
    else
    {
        // diffuse
        vec3 lightDir = normalize(vec4(u_lightPosition, 1).xyz - pos);
        specular = clamp(u_baseLight + dot(normal, lightDir) * u_baseLightDiff, 0, 1);

        // specular
        const float specularStrength = 0.25;
        const float shininess = 2;
        vec3 viewDir = normalize(-pos);
        vec3 reflectDir = reflect(-lightDir, normal);
        float specularAngle = max(dot(viewDir, reflectDir), 0.0);
        float spec = pow(specularAngle, shininess/4);
        specular += specularStrength * spec;
    }
    return specular;
}
