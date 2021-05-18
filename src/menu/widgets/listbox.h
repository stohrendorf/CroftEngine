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

namespace menu::widgets
{
class ListBox : public Widget
{
public:
  explicit ListBox(size_t pageSize, int width, const glm::ivec2& position);
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
  const size_t m_pageSize;
  const int m_width;
  glm::ivec2 m_position;
  size_t m_selected = 0;
  std::vector<std::shared_ptr<Widget>> m_widgets;
};
} // namespace menu::widgets
