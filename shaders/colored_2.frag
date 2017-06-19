uniform vec3 u_diffuseColor;

varying vec3 v_color;

out vec4 out_color;

#ifdef GOUREAUD
varying float v_shadeFactor;
#else
uniform vec3 u_lightPosition;
uniform float u_baseLight;
uniform float u_baseLightDiff;

varying vec3 v_vertexPos;
varying vec3 v_normal;
#endif

vec3 srgbEncode(in vec3 color){
   float r = color.r < 0.0031308 ? 12.92 * color.r : 1.055 * pow(color.r, 1.0/2.4) - 0.055;
   float g = color.g < 0.0031308 ? 12.92 * color.g : 1.055 * pow(color.g, 1.0/2.4) - 0.055;
   float b = color.b < 0.0031308 ? 12.92 * color.b : 1.055 * pow(color.b, 1.0/2.4) - 0.055;
   return vec3(r, g, b);
}

vec3 srgbDecode(in vec3 color){
   float r = color.r < 0.04045 ? (1.0 / 12.92) * color.r : pow((color.r + 0.055) * (1.0 / 1.055), 2.4);
   float g = color.g < 0.04045 ? (1.0 / 12.92) * color.g : pow((color.g + 0.055) * (1.0 / 1.055), 2.4);
   float b = color.b < 0.04045 ? (1.0 / 12.92) * color.b : pow((color.b + 0.055) * (1.0 / 1.055), 2.4);
   return vec3(r, g, b);
}

void main()
{
    vec3 color = srgbDecode(u_diffuseColor);

    out_color.r = color.r * v_color.r;
    out_color.g = color.g * v_color.g;
    out_color.b = color.b * v_color.b;
    out_color.a = 1;

    #ifdef GOUREAUD
    out_color *= v_shadeFactor;
#else
    float shadeFactor;
    if(isnan(u_lightPosition.x) || v_normal == vec3(0))
    {
        shadeFactor = clamp(u_baseLight + u_baseLightDiff, 0, 1);
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
#endif
    out_color.a = 1;
}
