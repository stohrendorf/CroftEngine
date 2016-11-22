vec4 getPosition()
{
    return a_position;
}

#if defined(LIGHTING)
  vec3 getNormal()
  {
      return a_normal;
  }
#endif
