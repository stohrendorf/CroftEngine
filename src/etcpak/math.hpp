#pragma once

#include <algorithm>
#include <cstdint>
#include <gsl-lite/gsl-lite.hpp>

constexpr uint8_t clampu8(const int32_t val)
{
  return gsl_lite::narrow_cast<uint8_t>(std::clamp(val, 0, 255));
}

constexpr uint8_t clampu8(const uint32_t val)
{
  return gsl_lite::narrow_cast<uint8_t>(std::clamp(val, 0u, 255u));
}

template<typename T>
constexpr T sq(T val)
{
  return val * val;
}