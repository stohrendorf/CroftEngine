#pragma once

#include <cstdint>
#include <gsl/gsl-lite.hpp>

template<typename T>
constexpr uint8_t clampu8(T val)
{
  if(val >= 0 && val <= 255)
    return gsl::narrow_cast<uint8_t>(val);
  else if(val < 0)
    return 0;
  else
    return 255;
}

constexpr uint8_t clampu8(uint8_t val)
{
  return val;
}

template<typename T>
constexpr T sq(T val)
{
  return val * val;
}
