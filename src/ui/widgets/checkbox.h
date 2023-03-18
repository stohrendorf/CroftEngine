#pragma once

#include "widget.h"

#include <glm/vec2.hpp>
#include <gslu.h>
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
class Checkbox : public Widget
{
public:
  explicit Checkbox(const std::string& label);
  explicit Checkbox(gslu::nn_shared<Widget> content);
  ~Checkbox() override;
  void update(bool hasFocus) override;
  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override;

  void setChecked(bool checked)
  {
    m_checked = checked;
  }

  void setPosition(const glm::ivec2& position) override;

  [[nodiscard]] glm::ivec2 getPosition() const override;
  [[nodiscard]] glm::ivec2 getSize() const override;
  void setSize(const glm::ivec2& size) override;
  void fitToContent() override;

  [[nodiscard]] const auto& getContent() const
  {
    return m_content;
  }

private:
  glm::ivec2 m_size{0, 0};
  gslu::nn_shared<Widget> m_content;
  bool m_checked = false;
};
} // namespace ui::widgets
