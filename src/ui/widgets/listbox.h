#pragma once

#include "widget.h"

#include <cstddef>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
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
  explicit ListBox(size_t pageSize = 0);
  ~ListBox() override;
  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override;

  size_t append(const gslu::nn_shared<Widget>& widget);

  [[nodiscard]] size_t getSelected() const
  {
    return m_selected;
  }

  bool nextEntry()
  {
    if(m_selected < m_widgets.size() - 1)
    {
      ++m_selected;
      return true;
    }
    return false;
  }

  bool prevEntry()
  {
    if(m_selected > 0)
    {
      --m_selected;
      return true;
    }
    return false;
  }

  bool nextPage()
  {
    if(m_selected + m_pageSize < m_widgets.size())
    {
      m_selected += m_pageSize;
      return true;
    }
    return false;
  }

  bool prevPage()
  {
    if(m_selected >= m_pageSize)
    {
      m_selected -= m_pageSize;
      return true;
    }
    return false;
  }

  void setSelected(size_t idx)
  {
    gsl_Expects(idx < m_widgets.size());
    m_selected = idx;
  }

  [[nodiscard]] glm::ivec2 getSize() const override;

  [[nodiscard]] glm::ivec2 getPosition() const override
  {
    return m_position;
  }

  void setPosition(const glm::ivec2& position) override;
  void setSize(const glm::ivec2& size) override;
  void update(bool hasFocus) override;
  void fitToContent() override;

  [[nodiscard]] const auto& getWidgets() const
  {
    return m_widgets;
  }

  [[nodiscard]] size_t getCurrentPage() const;
  [[nodiscard]] size_t getTotalPages() const;

  void clear()
  {
    m_widgets.clear();
  }

private:
  glm::ivec2 m_position{0, 0};
  glm::ivec2 m_size{0, 0};
  size_t m_pageSize;
  size_t m_selected = 0;
  std::vector<gslu::nn_shared<Widget>> m_widgets;
};
} // namespace ui::widgets
