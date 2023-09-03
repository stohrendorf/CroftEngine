#pragma once

#include "widget.h"

#include <cstddef>
#include <glm/vec2.hpp>
#include <memory>
#include <string>
#include <vector>

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
class GridBox;
class Label;

class SelectionBox : public Widget
{
public:
  explicit SelectionBox(const std::string& message,
                        const std::vector<std::string>& options,
                        size_t initialSelection = 0);
  explicit SelectionBox(const std::vector<std::string>& message,
                        const std::vector<std::string>& options,
                        size_t initialSelection = 0);
  ~SelectionBox() override;
  void update(bool hasFocus) override;
  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override;

  void setPosition(const glm::ivec2& position) override;

  [[nodiscard]] glm::ivec2 getPosition() const override;
  [[nodiscard]] glm::ivec2 getSize() const override;
  void setSize(const glm::ivec2& size) override;
  void fitToContent() override;

  [[nodiscard]] auto getSelected() const
  {
    return m_selected;
  }

  void prev()
  {
    if(m_selected > 0)
      --m_selected;
  }

  void next()
  {
    if(m_selected + 1 < m_options.size())
      ++m_selected;
  }

private:
  std::shared_ptr<GridBox> m_container;
  std::vector<std::shared_ptr<Label>> m_messageLines;
  std::vector<std::shared_ptr<Label>> m_options;
  size_t m_selected;
  glm::ivec2 m_position{0, 0};
  glm::ivec2 m_size{0, 0};
};
} // namespace ui::widgets
