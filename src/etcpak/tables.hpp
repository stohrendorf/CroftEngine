#pragma once

#include "ssevec.h"

#include <array>
#include <cstdint>

[[nodiscard]]
extern const std::array<std::array<int32_t, 4>, 8>& getTable();

[[nodiscard]]
extern const std::array<std::array<uint32_t, 16>, 4>& getId();

[[nodiscard]]
extern const std::array<uint32_t, 64>& getFlags();

[[nodiscard]]
extern const std::array<std::array<int16_t, 8>, 16>& getAlpha();

[[nodiscard]]
extern const std::array<IVec16, 2>& getTable128_SIMD();

[[nodiscard]]
extern const std::array<IVec16, 16>& getAlpha_SIMD();

[[nodiscard]]
extern const IVec16& getAlphaRange_SIMD();
