#pragma once

#include <cstdint>
#include <gsl/gsl-lite.hpp>

constexpr uint8_t clampu8(int32_t val)
{
  if((val & ~0xFF) == 0)
    return gsl::narrow_cast<uint8_t>(val);
  return gsl::narrow_cast<uint8_t>(((~val) >> 31) & 0xFF);
}

template<typename T>
constexpr T sq(T val)
{
  return val * val;
}
