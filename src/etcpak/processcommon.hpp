#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>

template<typename T, size_t N>
inline size_t getLeastError(const std::array<T, N>& err)
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

inline uint64_t fixByteOrder(uint64_t d)
{
  return ((d & 0x00000000FFFFFFFF)) | ((d & 0xFF00000000000000) >> 24) | ((d & 0x000000FF00000000) << 24)
         | ((d & 0x00FF000000000000) >> 8) | ((d & 0x0000FF0000000000) << 8);
}
