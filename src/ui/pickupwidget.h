#pragma once

#include "core/units.h"

#include <gl/cimgwrapper.h>
#include <gl/image.h>
#include <gl/pixel.h>

namespace ui
{
class PickupWidget
{
public:
  explicit PickupWidget(const core::Frame& duration, gl::CImgWrapper image)
      : m_duration{duration}
      , m_image{std::move(image)}
  {
  }

  bool expired() const
  {
    return m_duration <= 0_frame;
  }

  void draw(gl::Image<gl::SRGBA8>& dst, int x, int y) const
  {
    auto src{m_image};
    src.scale(3.0f / 16 * dst.getWidth() / 320);
    for(int dy = 0; dy < m_image.height(); ++dy)
      for(int dx = 0; dx < m_image.width(); ++dx)
        dst.set(glm::ivec2{x + dx, y + dy}, m_image(dx, dy), true);
  }

  void nextFrame()
  {
    m_duration -= 1_frame;
  }

private:
  core::Frame m_duration;
  mutable gl::CImgWrapper m_image;
};
} // namespace ui
