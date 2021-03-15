#pragma once

#include "core/units.h"
#include "loader/file/datatypes.h"
#include "ui/ui.h"

namespace ui
{
class PickupWidget
{
public:
  explicit PickupWidget(const core::Frame& duration, engine::world::Sprite sprite)
      : m_duration{duration}
      , m_sprite{std::move(sprite)}
  {
  }

  bool expired() const
  {
    return m_duration <= 0_frame;
  }

  void draw(ui::Ui& ui, int x, int y) const
  {
    ui.draw(m_sprite, {x, y});
  }

  void nextFrame()
  {
    m_duration -= 1_frame;
  }

private:
  core::Frame m_duration;
  mutable engine::world::Sprite m_sprite;
};
} // namespace ui
