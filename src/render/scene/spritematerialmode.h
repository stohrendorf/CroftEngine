#pragma once

#include <cstdint>

namespace render::scene
{
enum class SpriteMaterialMode : uint8_t
{
  YAxisBound = 1,
  Billboard = 2,
  InstancedBillboard = 3,
};
} // namespace render::scene
