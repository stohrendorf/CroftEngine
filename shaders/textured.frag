#version 150

#ifndef DIRECTIONAL_LIGHT_COUNT
# define DIRECTIONAL_LIGHT_COUNT 0
#endif

#ifndef SPOT_LIGHT_COUNT
# define SPOT_LIGHT_COUNT 0
#endif

#ifndef POINT_LIGHT_COUNT
# define POINT_LIGHT_COUNT 0
#endif

#if (DIRECTIONAL_LIGHT_COUNT > 0) || (POINT_LIGHT_COUNT > 0) || (SPOT_LIGHT_COUNT > 0)
# define LIGHTING
#endif

///////////////////////////////////////////////////////////
// Uniforms
uniform vec3 u_ambientColor;

uniform sampler2D u_diffuseTexture;

#if defined(LIGHTING)

  #if (DIRECTIONAL_LIGHT_COUNT > 0)
    uniform vec3 u_directionalLightColor[DIRECTIONAL_LIGHT_COUNT];
    uniform vec3 u_directionalLightDirection[DIRECTIONAL_LIGHT_COUNT];
  #endif

  #if (POINT_LIGHT_COUNT > 0)
    uniform vec3 u_pointLightColor[POINT_LIGHT_COUNT];
    uniform vec3 u_pointLightPosition[POINT_LIGHT_COUNT];
    uniform float u_pointLightRangeInverse[POINT_LIGHT_COUNT];
  #endif

  #if (SPOT_LIGHT_COUNT > 0)
    uniform vec3 u_spotLightColor[SPOT_LIGHT_COUNT];
    uniform float u_spotLightRangeInverse[SPOT_LIGHT_COUNT];
    uniform float u_spotLightInnerAngleCos[SPOT_LIGHT_COUNT];
    uniform float u_spotLightOuterAngleCos[SPOT_LIGHT_COUNT];
    uniform vec3 u_spotLightDirection[SPOT_LIGHT_COUNT];
  #endif

  #if defined(SPECULAR)
    uniform float u_specularExponent;
  #endif

#endif // LIGHTING

#if defined(MODULATE_COLOR)
  uniform vec4 u_modulateColor;
#endif

#if defined(MODULATE_ALPHA)
  uniform float u_modulateAlpha;
#endif

///////////////////////////////////////////////////////////
// Variables
vec4 _baseColor;

///////////////////////////////////////////////////////////
// Varyings
varying vec2 v_texCoord;

#if defined(LIGHTING)

  varying vec3 v_normalVector;

  #if (POINT_LIGHT_COUNT > 0)
    varying vec3 v_vertexToPointLightDirection[POINT_LIGHT_COUNT];
  #endif

  #if (SPOT_LIGHT_COUNT > 0)
    varying vec3 v_vertexToSpotLightDirection[SPOT_LIGHT_COUNT];
  #endif

  #if defined(SPECULAR)
    varying vec3 v_cameraDirection;
  #endif

  #include "lighting.frag"

#endif // LIGHTING

#if defined(CLIP_PLANE)
  varying float v_clipDistance;
#endif


void main()
{
  #if defined(CLIP_PLANE)
    if(v_clipDistance < 0.0)
      discard;
  #endif

  _baseColor = texture2D(u_diffuseTexture, v_texCoord);

  gl_FragColor.a = _baseColor.a;

  #if defined(TEXTURE_DISCARD_ALPHA)
    if (gl_FragColor.a < 0.5)
      discard;
  #endif

  #if defined(LIGHTING)
    gl_FragColor.rgb = getLitPixel();
  #else
    gl_FragColor.rgb = _baseColor.rgb;
  #endif

  #if defined(MODULATE_COLOR)
    gl_FragColor *= u_modulateColor;
  #endif

  #if defined(MODULATE_ALPHA)
    gl_FragColor.a *= u_modulateAlpha;
  #endif
}
