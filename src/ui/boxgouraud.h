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
    result.topLeft.channels[3] = result.topLeft.channels[3] * alpha / 255;
    result.topRight.channels[3] = result.topRight.channels[3] * alpha / 255;
    result.bottomRight.channels[3] = result.bottomRight.channels[3] * alpha / 255;
    result.bottomLeft.channels[3] = result.bottomLeft.channels[3] * alpha / 255;
    return result;
  }
};
} // namespace ui
