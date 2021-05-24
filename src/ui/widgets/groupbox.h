#pragma once

#include "ui/core.h"
#include "widget.h"

#include <glm/glm.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <string>
#include <vector>

namespace engine
{
class Presenter;
}

namespace ui
{
class Ui;
struct Label;
} // namespace ui

namespace ui::widgets
{
class GroupBox : public Widget
{
public:
  explicit GroupBox(const glm::ivec2& position,
                    const glm::ivec2& size,
                    const std::string& title,
                    gsl::not_null<std::shared_ptr<Widget>> widget);
  ~GroupBox() override;
  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override;

  [[nodiscard]] glm::ivec2 getSize() const override;

  [[nodiscard]] glm::ivec2 getPosition() const override
  {
    return m_position;
  }

  void setPosition(const glm::ivec2& position) override;
  void setSize(const glm::ivec2& size) override;
  void update(bool hasFocus) override;
  void fitToContent() override;

private:
  glm::ivec2 m_position;
  glm::ivec2 m_size;
  gsl::not_null<std::unique_ptr<ui::Label>> m_title;
  gsl::not_null<std::shared_ptr<Widget>> m_widget;
};
} // namespace ui::widgets
