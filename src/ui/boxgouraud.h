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
    const auto multiplyAlpha = [alpha](uint8_t& value) { value = value * alpha / 255; };

    auto result = *this;
    multiplyAlpha(result.topLeft.channels[3]);
    multiplyAlpha(result.topRight.channels[3]);
    multiplyAlpha(result.bottomRight.channels[3]);
    multiplyAlpha(result.bottomLeft.channels[3]);
    return result;
  }

  [[nodiscard]] auto mirroredY() const
  {
    return BoxGouraud{
      bottomLeft,
      bottomRight,
      topRight,
      topLeft,
    };
  }
};
} // namespace ui
