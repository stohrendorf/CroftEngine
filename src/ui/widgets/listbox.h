#pragma once

#include "ui/core.h"
#include "widget.h"

#include <glm/glm.hpp>
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
} // namespace ui

namespace ui::widgets
{
class ListBox : public Widget
{
public:
  explicit ListBox(const glm::ivec2& position, const glm::ivec2& size, size_t pageSize);
  ~ListBox() override;
  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override;

  size_t addEntry(const std::shared_ptr<Widget>& widget);

  [[nodiscard]] size_t getSelected() const
  {
    return m_selected;
  }

  void nextEntry()
  {
    if(m_selected < m_widgets.size() - 1)
      ++m_selected;
  }

  void prevEntry()
  {
    if(m_selected > 0)
      --m_selected;
  }

  void nextPage()
  {
    if(m_selected + m_pageSize < m_widgets.size())
      m_selected += m_pageSize;
  }

  void prevPage()
  {
    if(m_selected >= m_pageSize)
      m_selected -= m_pageSize;
  }

  [[nodiscard]] glm::ivec2 getSize() const override;

  [[nodiscard]] glm::ivec2 getPosition() const override
  {
    return m_position;
  }

  void setPosition(const glm::ivec2& position) override;

  void update(bool hasFocus) override;

private:
  glm::ivec2 m_position;
  glm::ivec2 m_size;
  const size_t m_pageSize;
  size_t m_selected = 0;
  std::vector<std::shared_ptr<Widget>> m_widgets;
};
} // namespace ui::widgets
