#pragma once

#include "widget.h"

#include <cstdint>
#include <glm/vec2.hpp>
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
class Text;
} // namespace ui

namespace ui::widgets
{
class TextBox : public Widget
{
public:
  enum class Alignment : uint8_t
  {
    Left,
    Center,
    Right
  };

  explicit TextBox(const std::string& text, int maxWidth, Alignment alignment = Alignment::Left);
  ~TextBox() override;

  [[nodiscard]] glm::ivec2 getPosition() const override;
  [[nodiscard]] glm::ivec2 getSize() const override;
  void setPosition(const glm::ivec2& position) override;
  void setSize(const glm::ivec2& size) override;
  void update(bool hasFocus) override;
  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override;
  void fitToContent() override;

  void setText(const std::string& text);

private:
  std::string m_text;
  std::vector<std::unique_ptr<ui::Text>> m_lines;
  glm::ivec2 m_position{0, 0};
  glm::ivec2 m_size;
  Alignment m_alignment = Alignment::Left;
};
} // namespace ui::widgets
