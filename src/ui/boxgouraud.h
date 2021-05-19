#pragma once

#include <gl/pixel.h>

namespace ui
{
struct BoxGouraud
{
  gl::SRGBA8 topLeft;
  gl::SRGBA8 topRight;
  gl::SRGBA8 bottomRight;
  gl::SRGBA8 bottomLeft;

  [[nodiscard]] auto withAlpha(uint8_t alpha) const
  {
    auto result = *this;
    result.topLeft.channels[3] = alpha;
    result.topRight.channels[3] = alpha;
    result.bottomRight.channels[3] = alpha;
    result.bottomLeft.channels[3] = alpha;
    return result;
  }
};
} // namespace ui
