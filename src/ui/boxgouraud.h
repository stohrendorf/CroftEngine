#pragma once

#include "ui.h"

#include <gl/pixel.h>

namespace ui
{
struct BoxGouraud
{
  gl::SRGBA8 topLeft;
  gl::SRGBA8 topRight;
  gl::SRGBA8 bottomRight;
  gl::SRGBA8 bottomLeft;

  [[nodiscard]] auto mirroredY() const noexcept
  {
    return BoxGouraud{
      bottomLeft,
      bottomRight,
      topRight,
      topLeft,
    };
  }
};

struct BackgroundGouraud
{
  BoxGouraud topLeft;
  BoxGouraud topRight;
  BoxGouraud bottomRight;
  BoxGouraud bottomLeft;

  [[nodiscard]] auto mirroredY() const
  {
    return BackgroundGouraud{
      bottomLeft.mirroredY(),
      bottomRight.mirroredY(),
      topRight.mirroredY(),
      topLeft.mirroredY(),
    };
  }

  void draw(Ui& ui, const glm::ivec2& pos, const glm::ivec2& size) const
  {
    const auto half = size / 2;
    const auto half2 = size - half;
    ui.drawBox(pos, half, topLeft);
    ui.drawBox(pos + glm::ivec2{half.x, 0}, {half2.x, half.y}, topRight);
    ui.drawBox(pos + half, {half.x, half2.y}, bottomRight);
    ui.drawBox(pos + glm::ivec2{0, half.y}, {half2.x, half2.y}, bottomLeft);
  }
};

inline BackgroundGouraud makeBackgroundCircle(const gl::SRGBA8& center, const gl::SRGBA8& outer) noexcept
{
  return {BoxGouraud{outer, outer, center, outer},
          BoxGouraud{outer, outer, outer, center},
          BoxGouraud{center, outer, outer, outer},
          BoxGouraud{outer, center, outer, outer}};
}

inline BackgroundGouraud makeBackgroundCircle(const gl::SRGB8& color, uint8_t innerAlpha, uint8_t outerAlpha)
{
  return makeBackgroundCircle(gl::SRGBA8{color.channels[0], color.channels[1], color.channels[2], innerAlpha},
                              gl::SRGBA8{color.channels[0], color.channels[1], color.channels[2], outerAlpha});
}
} // namespace ui
