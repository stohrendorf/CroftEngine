attribute vec3 a_position;
attribute vec3 a_normal;
// attribute vec2 a_texCoord;
attribute vec3 a_color;

uniform mat4 u_worldViewProjectionMatrix;
uniform mat4 u_modelMatrix;
uniform mat4 u_viewMatrix;
uniform vec3 u_lightPosition;

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
        v_shadeFactor = 1;
        return;
    }

    vec3 camSpacePos = (u_viewMatrix * u_modelMatrix * vec4(a_position, 1)).xyz;

    // Normal of the computed fragment, in camera space
    vec3 n = normalize((u_viewMatrix * u_modelMatrix * vec4(a_normal, 0)).xyz);
    // Direction of the light (from the fragment to the light)
    vec3 dir = normalize((u_viewMatrix * vec4(u_lightPosition, 1)).xyz - camSpacePos);

    v_shadeFactor = clamp(dot(n, dir), 0, 1);
}
