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
class HBox : public Widget
{
public:
  explicit HBox(const glm::ivec2& position, const glm::ivec2& size);
  ~HBox() override;
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
  size_t m_selected = 0;
  std::vector<gsl::not_null<std::shared_ptr<Widget>>> m_widgets;
};
} // namespace ui::widgets
