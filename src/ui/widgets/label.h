#pragma once

#include "widget.h"

#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace ui
{
class Text;
}

namespace ui::widgets
{
class Label : public Widget
{
public:
  explicit Label(const std::string& text);
  ~Label() override;

  [[nodiscard]] glm::ivec2 getPosition() const override;
  [[nodiscard]] glm::ivec2 getSize() const override;
  void setPosition(const glm::ivec2& position) override;
  void setSize(const glm::ivec2& size) override;
  void update(bool hasFocus) override;
  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override;
  void fitToContent() override;

  void setText(const std::string& text);

private:
  std::unique_ptr<ui::Text> m_text;
  glm::ivec2 m_position{0, 0};
  glm::ivec2 m_size;
  uint8_t m_selectionAlpha = 0;
};
} // namespace ui::widgets
