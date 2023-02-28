#pragma once

namespace render::material
{
enum class RenderMode
{
  FullOpaque,
  FullNonOpaque,
  CSMDepthOnly,
  DepthOnly
};
} // namespace render::material
