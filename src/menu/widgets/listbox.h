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
class ListBox
{
public:
  static constexpr int EntryHeight = 18;
  static constexpr int BottomMargin = 80;

  explicit ListBox(size_t pageSize = 10, int pixelWidth = 272);
  void draw(ui::Ui& ui, const engine::Presenter& presenter);

  size_t addEntry(const std::string& label);

  void setActive(size_t idx, bool active);
  [[nodiscard]] size_t getSelected() const
  {
    return m_selected;
  }

  void nextEntry()
  {
    if(m_selected < m_labels.size() - 1)
      ++m_selected;
  }

  void prevEntry()
  {
    if(m_selected > 0)
      --m_selected;
  }

  void nextPage()
  {
    if(m_selected + m_pageSize < m_labels.size())
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

  [[nodiscard]] auto getPageSize() const
  {
    return m_pageSize;
  }

  [[nodiscard]] auto getHeight() const
  {
    return m_pageSize * EntryHeight + 10;
  }

  [[nodiscard]] auto getTop() const
  {
    return -BottomMargin - getHeight();
  }

private:
  const size_t m_pageSize;
  const int m_pixelWidth;
  size_t m_selected = 0;
  std::vector<std::tuple<std::unique_ptr<ui::Label>, bool>> m_labels;
};
} // namespace menu::widgets
