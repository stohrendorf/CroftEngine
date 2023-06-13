#include "sprite.h"

#include "ui/core.h"
#include "ui/ui.h"

#include <algorithm>
#include <gl/pixel.h>
#include <glm/common.hpp>
#include <gsl/gsl-lite.hpp>
#include <utility>

namespace ui::widgets
{
Sprite::Sprite(engine::world::Sprite sprite)
    : m_sprite{std::move(sprite)}
{
}

Sprite::~Sprite() = default;

void Sprite::draw(ui::Ui& ui, const engine::Presenter& /*presenter*/) const
{
  if(m_selectionAlpha != 0)
  {
    const auto bgPos = m_position - glm::ivec2{OutlineBorderWidth * 3 / 2, FontHeight};
    const auto bgSize = m_size + glm::ivec2{2 * OutlineBorderWidth, 0};
    ui.drawBox(bgPos, bgSize, gl::SRGBA8{0, 0, 0, DefaultBackgroundAlpha * m_selectionAlpha / 255});
    ui.drawOutlineBox(bgPos, bgSize, m_selectionAlpha);
  }
  ui.draw(m_sprite, m_position);
}

void Sprite::update(bool hasFocus)
{
  constexpr int FadeSpeed = 30;
  const auto delta = hasFocus ? FadeSpeed : -FadeSpeed;
  m_selectionAlpha = gsl::narrow_cast<uint8_t>(std::clamp(m_selectionAlpha + delta, 0, 255));
}

void Sprite::setPosition(const glm::ivec2& position)
{
  m_position = position;
}

glm::ivec2 Sprite::getPosition() const
{
  return m_position;
}

glm::ivec2 Sprite::getSize() const
{
  return m_size;
}

void Sprite::setSize(const glm::ivec2& size)
{
  m_size = size;
}

void Sprite::fitToContent()
{
  m_size = glm::max(m_sprite.render0, m_sprite.render1) + glm::ivec2{0, ui::FontHeight};
}
} // namespace ui::widgets
