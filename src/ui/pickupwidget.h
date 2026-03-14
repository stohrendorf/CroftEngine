#pragma once

#include "core.h"
#include "core/units.h"
#include "text.h"
#include "ui/ui.h"

#include <cstddef>
#include <string>
#include <utility>

namespace ui
{
class PickupWidget
{
public:
  explicit PickupWidget(const core::Tick& duration, engine::world::Sprite sprite, size_t count)
      : m_duration{duration}
      , m_sprite{std::move(sprite)}
      , m_count{count}
  {
  }

  bool expired() const
  {
    return m_duration <= 0_tick;
  }

  void draw(Ui& ui, int x, int y, const TRFont& font, const float scale) const
  {
    ui.draw(m_sprite, {x, y}, scale);
    if(m_count <= 1)
      return;

    const auto txt = Text{std::to_string(m_count)};
    drawBox(txt, ui, {x, y}, FontHeight / 4, gl::SRGBA8{0, 0, 0, 192});
    txt.draw(ui, font, {x, y});
  }

  void tick()
  {
    m_duration -= 1_tick;
  }

  [[nodiscard]] auto getWidth() const
  {
    return m_sprite.render1.x - m_sprite.render0.x;
  }

  [[nodiscard]] auto getDuration() const
  {
    return m_duration;
  }

  [[nodiscard]] const auto& getSprite() const
  {
    return m_sprite;
  }

  void setCount(const size_t val)
  {
    m_count = val;
  }

private:
  core::Tick m_duration;
  mutable engine::world::Sprite m_sprite;
  size_t m_count;
};
} // namespace ui
