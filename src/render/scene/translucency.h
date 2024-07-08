#pragma once

#include <cstdint>

namespace render::scene
{
enum class Translucency : uint8_t
{
  Opaque,
  NonOpaque
};
} // namespace render::scene
