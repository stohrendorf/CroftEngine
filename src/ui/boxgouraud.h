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
};
} // namespace ui
