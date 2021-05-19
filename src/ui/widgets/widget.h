#pragma once

#include <glm/fwd.hpp>

namespace ui
{
class Ui;
}

namespace engine
{
class Presenter;
}

namespace ui::widgets
{
class Widget
{
public:
  virtual ~Widget() = default;

  [[nodiscard]] virtual glm::ivec2 getPosition() const = 0;
  [[nodiscard]] virtual glm::ivec2 getSize() const = 0;
  virtual void setPosition(const glm::ivec2& position) = 0;
  virtual void setSize(const glm::ivec2& size) = 0;
  virtual void update(bool hasFocus) = 0;

  virtual void draw(ui::Ui& ui, const engine::Presenter& presenter) const = 0;
};
} // namespace ui::widgets
