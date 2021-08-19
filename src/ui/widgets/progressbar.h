#pragma once

#include "widget.h"

#include <glm/glm.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <string>

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
class ProgressBar : public Widget
{
public:
  explicit ProgressBar() = default;
  ~ProgressBar() override;
  void update(bool hasFocus) override;
  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override;

  void setValue(float value)
  {
    m_value = value;
  }

  [[nodiscard]] float getValue() const
  {
    return m_value;
  }

  void setPosition(const glm::ivec2& position) override;

  [[nodiscard]] glm::ivec2 getPosition() const override;
  [[nodiscard]] glm::ivec2 getSize() const override;
  void setSize(const glm::ivec2& size) override;
  void fitToContent() override;

private:
  glm::ivec2 m_position{0, 0};
  glm::ivec2 m_size{0, 0};
  float m_value = 0;
};
} // namespace ui::widgets
