attribute vec3 a_position;
attribute vec3 a_normal;
// attribute vec2 a_texCoord;
attribute vec3 a_color;

uniform mat4 u_worldViewProjectionMatrix;
uniform mat4 u_modelMatrix;
uniform vec3 u_lightPosition;
uniform float u_baseLight;
uniform float u_baseLightDiff;

// varying vec2 v_texCoord;
varying vec3 v_color;
varying float v_shadeFactor;

void main()
{
    gl_Position = u_worldViewProjectionMatrix * vec4(a_position, 1);
    // v_texCoord = a_texCoord;
    v_color = a_color;

    if(isnan(u_lightPosition.x))
    {
        v_shadeFactor = clamp(u_baseLight + u_baseLightDiff, 0, 1);
        return;
    }

    vec3 vertexPos = (u_modelMatrix * vec4(a_position, 1)).xyz;
    vec3 n = normalize((u_modelMatrix * vec4(a_normal, 0)).xyz);
    vec3 dir = normalize(vec4(u_lightPosition, 1).xyz - vertexPos);

    v_shadeFactor = clamp(u_baseLight + dot(n, dir) * u_baseLightDiff, 0, 1);
}
