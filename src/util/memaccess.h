#pragma once

#include <cstdint>
#include <gsl/gsl-lite.hpp>

namespace util
{
template<typename T>
constexpr T readUnalignedLE(const uint8_t* data)
{
  gsl_Expects(data != nullptr);

  T result{0};
  for(size_t i = 0; i < sizeof(T); ++i)
  {
    result |= static_cast<T>(data[i]) << (8u * i);
  }
  return result;
}

constexpr uint32_t readUnaligned32LE(const uint8_t* data)
{
  return readUnalignedLE<uint32_t>(data);
}
} // namespace util
