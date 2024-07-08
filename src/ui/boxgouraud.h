#pragma once

#include "core.h"
#include "ui.h"

#include <gl/pixel.h>
#include <glm/vec2.hpp>

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

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
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

constexpr gl::SRGBA8 UiBackdropCenterColor{0, 128, 0, DefaultBackgroundAlpha};
constexpr gl::SRGBA8 UiBackdropOuterColor{0, 0, 0, DefaultBackgroundAlpha};
constexpr BackgroundGouraud UiBackdrop{
  BoxGouraud{UiBackdropOuterColor, UiBackdropOuterColor, UiBackdropCenterColor, UiBackdropOuterColor},
  BoxGouraud{UiBackdropOuterColor, UiBackdropOuterColor, UiBackdropOuterColor, UiBackdropCenterColor},
  BoxGouraud{UiBackdropCenterColor, UiBackdropOuterColor, UiBackdropOuterColor, UiBackdropOuterColor},
  BoxGouraud{UiBackdropOuterColor, UiBackdropCenterColor, UiBackdropOuterColor, UiBackdropOuterColor}};
} // namespace ui
