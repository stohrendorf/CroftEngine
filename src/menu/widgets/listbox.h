#pragma once

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

namespace menu::widgets
{
class Checkbox;

class ListBox
{
public:
  static constexpr int EntryHeight = 18;

  explicit ListBox(size_t pageSize, int pixelWidth, int bottomMargin);
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

  [[nodiscard]] auto getWidth() const
  {
    return m_pixelWidth;
  }

  [[nodiscard]] auto getHeight() const
  {
    return m_pageSize * EntryHeight;
  }

  [[nodiscard]] auto getTop() const
  {
    return -m_bottomMargin - getHeight();
  }

private:
  const size_t m_pageSize;
  const int m_pixelWidth;
  const int m_bottomMargin;
  size_t m_selected = 0;
  std::vector<std::unique_ptr<widgets::Checkbox>> m_checkboxes;
};
} // namespace menu::widgets
