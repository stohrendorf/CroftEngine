attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec3 a_color;

uniform mat4 u_modelMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_projectionMatrix;

varying vec3 v_color;

#ifdef GOUREAUD
uniform vec3 u_lightPosition;
uniform float u_baseLight;
uniform float u_baseLightDiff;

varying float v_shadeFactor;
#else
varying vec3 v_vertexPos;
varying vec3 v_normal;
#endif

void main()
{
    gl_Position = u_projectionMatrix * u_modelViewMatrix * vec4(a_position, 1);
    v_color = a_color;

#ifdef GOUREAUD
    if(isnan(u_lightPosition.x) || a_normal == vec3(0))
    {
        v_shadeFactor = clamp(u_baseLight + u_baseLightDiff, 0, 1);
    }
    else
    {
        vec3 vertexPos = (u_modelMatrix * vec4(a_position, 1)).xyz;
        vec3 n = normalize((u_modelMatrix * vec4(a_normal, 0)).xyz);
        vec3 dir = normalize(vec4(u_lightPosition, 1).xyz - vertexPos);

        v_shadeFactor = clamp(u_baseLight + dot(n, dir) * u_baseLightDiff, 0, 1);
    }
#else
    v_vertexPos = (u_modelMatrix * vec4(a_position, 1)).xyz;
    v_normal = normalize((u_modelMatrix * vec4(a_normal, 0)).xyz);
#endif
}
