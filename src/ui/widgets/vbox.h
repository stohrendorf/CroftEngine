#pragma once

#include "ui/core.h"
#include "widget.h"

#include <glm/glm.hpp>
#include <gsl/gsl-lite.hpp>
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
class VBox : public Widget
{
public:
  explicit VBox(const glm::ivec2& position, const glm::ivec2& size, size_t pageSize = 0);
  ~VBox() override;
  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override;

  size_t append(const gsl::not_null<std::shared_ptr<Widget>>& widget);

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

  [[nodiscard]] glm::ivec2 getSize() const override;

  [[nodiscard]] glm::ivec2 getPosition() const override
  {
    return m_position;
  }

  void setPosition(const glm::ivec2& position) override;
  void setSize(const glm::ivec2& size) override;
  void update(bool hasFocus) override;
  void fitToContent() override;

private:
  glm::ivec2 m_position;
  glm::ivec2 m_size;
  const size_t m_pageSize;
  size_t m_selected = 0;
  std::vector<gsl::not_null<std::shared_ptr<Widget>>> m_widgets;
};
} // namespace ui::widgets
