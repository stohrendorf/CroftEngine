#include "sprite.h"

#include "engine/presenter.h"
#include "render/scene/sprite.h"
#include "ui/ui.h"

#include <utility>

namespace ui::widgets
{
Sprite::Sprite(const glm::ivec2& position, const glm::ivec2& size, engine::world::Sprite sprite)
    : m_position{position}
    , m_size{size}
    , m_sprite{std::move(sprite)}
{
}

Sprite::~Sprite() = default;

void Sprite::draw(ui::Ui& ui, const engine::Presenter& /*presenter*/) const
{
  ui.draw(m_sprite, m_position);
}

void Sprite::update(bool /*hasFocus*/)
{
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
  m_size = glm::max(m_sprite.render0, m_sprite.render1);
}
} // namespace ui::widgets
