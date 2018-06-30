uniform sampler2D u_diffuseTexture;
uniform vec3 u_lightPosition;
uniform float u_baseLight;
uniform float u_baseLightDiff;

in vec2 v_texCoord;
in vec3 v_color;
in vec3 v_vertexPos;
in vec3 v_normal;

out vec4 out_color;

void main()
{
    vec4 baseColor = texture2D(u_diffuseTexture, v_texCoord);

    if(baseColor.a < 0.5)
        discard;

    out_color.r = baseColor.r * v_color.r;
    out_color.g = baseColor.g * v_color.g;
    out_color.b = baseColor.b * v_color.b;
    out_color.a = baseColor.a;

#ifdef WATER
    const vec4 WaterColor = vec4(149 / 255.0f, 229 / 255.0f, 229 / 255.0f, 1);

    out_color *= WaterColor;
#endif

    float shadeFactor;
    if(isnan(u_lightPosition.x) || v_normal == vec3(0))
    {
        shadeFactor = clamp(u_baseLight, 0, 1);
    }
    else
    {
        // diffuse
        vec3 dir = normalize(vec4(u_lightPosition, 1).xyz - v_vertexPos);
        shadeFactor = clamp(u_baseLight + dot(v_normal, dir) * u_baseLightDiff, 0, 1);

        // specular
        const float specularStrength = 0.5;
        const float specularPower = 4;
        vec3 viewDir = normalize(-v_vertexPos);
        vec3 reflectDir = reflect(-dir, v_normal);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), specularPower);
        shadeFactor += specularStrength * spec;
    }
    out_color *= shadeFactor;
    out_color.a = 1;
}
