#pragma once

#include "widget.h"

#include <glm/fwd.hpp>
#include <memory>
#include <string>

namespace ui
{
class Label;
}

namespace ui::widgets
{
class Label : public Widget
{
public:
  explicit Label(const glm::ivec2& position, const std::string& label, int width = 0);
  ~Label() override;

  [[nodiscard]] glm::ivec2 getPosition() const override;
  [[nodiscard]] glm::ivec2 getSize() const override;
  void setPosition(const glm::ivec2& position) override;
  void setSize(const glm::ivec2& size) override;
  void update(bool hasFocus) override;
  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override;

private:
  std::unique_ptr<ui::Label> m_label;
};
} // namespace ui::widgets
