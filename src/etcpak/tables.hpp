#pragma once

#include "ssevec.h"

#include <array>
#include <cstdint>

extern const std::array<std::array<int32_t, 4>, 8> g_table;

extern const std::array<std::array<uint32_t, 16>, 4> g_id;

extern const std::array<uint32_t, 64> g_flags;

extern const std::array<std::array<int32_t, 8>, 16> g_alpha;
extern const std::array<int32_t, 16> g_alphaRange;

extern const std::array<IVec16, 2> g_table128_SIMD;

extern const std::array<IVec16, 16> g_alpha_SIMD;
extern const IVec16 g_alphaRange_SIMD;
