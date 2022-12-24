#pragma once

#include "forceinline.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

etcpak_force_inline uint8_t clampu8(int32_t val)
{
  if((val & ~0xFF) == 0)
    return val;
  return ((~val) >> 31) & 0xFF;
}

template<typename T>
etcpak_force_inline T sq(T val)
{
  return val * val;
}
