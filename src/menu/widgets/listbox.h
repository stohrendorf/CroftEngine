#pragma once

#include "ui/core.h"

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
class Checkbox;

class ListBox
{
public:
  static constexpr int EntryHeight = ui::FontHeight + ui::OutlineBorderWidth;

  explicit ListBox(size_t pageSize, int width, const glm::ivec2& position);
  ~ListBox();
  void draw(ui::Ui& ui, const engine::Presenter& presenter);

  size_t addEntry(const std::string& label);

  void setChecked(size_t idx, bool checked);
  [[nodiscard]] size_t getSelected() const
  {
    return m_selected;
  }

  void nextEntry()
  {
    if(m_selected < m_checkboxes.size() - 1)
      ++m_selected;
  }

  void prevEntry()
  {
    if(m_selected > 0)
      --m_selected;
  }

  void nextPage()
  {
    if(m_selected + m_pageSize < m_checkboxes.size())
      m_selected += m_pageSize;
  }

  void prevPage()
  {
    if(m_selected >= m_pageSize)
      m_selected -= m_pageSize;
  }

  [[nodiscard]] auto getSize() const
  {
    return glm::ivec2{m_width, m_pageSize * EntryHeight};
  }

  [[nodiscard]] const auto& getPosition() const
  {
    return m_position;
  }

  void setPosition(const glm::ivec2& position);

private:
  const size_t m_pageSize;
  const int m_width;
  glm::ivec2 m_position;
  size_t m_selected = 0;
  std::vector<std::unique_ptr<widgets::Checkbox>> m_checkboxes;
};
} // namespace menu::widgets
