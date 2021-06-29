#pragma once

#include "engine/world/sprite.h"
#include "widget.h"

#include <glm/glm.hpp>
#include <memory>

namespace ui
{
class Ui;
} // namespace ui

namespace engine
{
class Presenter;
}

namespace ui::widgets
{
class Label;

class Sprite : public Widget
{
public:
  explicit Sprite(engine::world::Sprite sprite);
  ~Sprite() override;
  void update(bool hasFocus) override;
  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override;

  void setPosition(const glm::ivec2& position) override;

  [[nodiscard]] glm::ivec2 getPosition() const override;
  [[nodiscard]] glm::ivec2 getSize() const override;
  void setSize(const glm::ivec2& size) override;
  void fitToContent() override;

private:
  glm::ivec2 m_position{0, 0};
  glm::ivec2 m_size{0, 0};
  engine::world::Sprite m_sprite;
  uint8_t m_selectionAlpha = 0;
};
} // namespace ui::widgets
