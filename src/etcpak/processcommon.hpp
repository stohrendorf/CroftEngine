#pragma once

#include <cstddef>
#include <cstdint>

template<typename T, size_t N>
constexpr inline size_t getLeastError(const std::array<T, N>& err)
{
  size_t idx = 0;
  for(size_t i = 1; i < N; i++)
  {
    if(err[i] < err[idx])
    {
      idx = i;
    }
  }
  return idx;
}

constexpr inline uint64_t fixByteOrder(uint64_t d)
{
  return ((d & 0x00000000FFFFFFFFu)) | ((d & 0xFF00000000000000u) >> 24u) | ((d & 0x000000FF00000000u) << 24u)
         | ((d & 0x00FF000000000000u) >> 8u) | ((d & 0x0000FF0000000000u) << 8u);
}
