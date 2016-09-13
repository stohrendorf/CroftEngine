void applyLight(vec4 position)
{
  #if defined(SPECULAR) || (POINT_LIGHT_COUNT > 0) || (SPOT_LIGHT_COUNT > 0)
    vec4 positionWorldViewSpace = u_worldViewMatrix * position;
  #endif

  #if (POINT_LIGHT_COUNT > 0)
    for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
    {
      // Compute the light direction with light position and the vertex position.
      v_vertexToPointLightDirection[i] = u_pointLightPosition[i] - positionWorldViewSpace.xyz;
    }
  #endif

  #if (SPOT_LIGHT_COUNT > 0)
    for (int i = 0; i < SPOT_LIGHT_COUNT; ++i)
    {
      // Compute the light direction with light position and the vertex position.
      v_vertexToSpotLightDirection[i] = u_spotLightPosition[i] - positionWorldViewSpace.xyz;
    }
  #endif

  #if defined(SPECULAR)
    v_cameraDirection = u_cameraPosition - positionWorldViewSpace.xyz;
  #endif
}
