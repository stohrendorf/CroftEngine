#pragma once

#include <cstdint>

namespace render::material
{
enum class RenderMode : uint8_t
{
  FullOpaque,
  FullNonOpaque,
  CSMDepthOnly,
  DepthOnly
};
} // namespace render::material
