#pragma once

#include "ui/core.h"
#include "widget.h"

#include <boost/multi_array.hpp>
#include <glm/glm.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <string>
#include <tuple>

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
class GridBox : public Widget
{
public:
  explicit GridBox(const glm::ivec2& position,
                   const glm::ivec2& size,
                   const glm::ivec2& separation = glm::ivec2{ui::OutlineBorderWidth, ui::OutlineBorderWidth});
  ~GridBox() override;
  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override;

  void set(size_t x, size_t y, const gsl::not_null<std::shared_ptr<Widget>>& widget);

  [[nodiscard]] const auto& getSelected() const
  {
    return m_selected;
  }

  void setSelected(const std ::pair<size_t, size_t>& selected)
  {
    m_selected = selected;
  }

  template<size_t N>
  bool next()
  {
    static_assert(N < 2);
    if(std::get<N>(m_selected) < m_widgets.shape()[N] - 1)
    {
      ++std::get<N>(m_selected);
      return true;
    }
    return false;
  }

  bool nextRow()
  {
    return next<1>();
  }

  bool nextColumn()
  {
    return prev<0>();
  }

  template<size_t N>
  bool prev()
  {
    static_assert(N < 2);
    if(std::get<N>(m_selected) > 0)
    {
      --std::get<N>(m_selected);
      return true;
    }
    return false;
  }

  bool prevRow()
  {
    return prev<1>();
  }

  bool prevColumn()
  {
    return prev<0>();
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

  void setExtents(size_t x, size_t y)
  {
    m_widgets.resize(boost::extents[x][y]);
    m_columnSizes.resize(x, 0);
    m_rowSizes.resize(y, 0);
    recalculateTotalSize();
  }

  [[nodiscard]] std::tuple<size_t, size_t> getExtents() const
  {
    return {
      m_widgets.shape()[0],
      m_widgets.shape()[1],
    };
  }

  void setRowSize(size_t row, int height)
  {
    m_rowSizes.at(row) = height;
    recalculateTotalSize();
  }

  void setColumnSize(size_t column, int width)
  {
    m_columnSizes.at(column) = width;
    recalculateTotalSize();
  }

  [[nodiscard]] const auto& getColumnSizes() const
  {
    return m_columnSizes;
  }

private:
  void recalculateTotalSize();

  glm::ivec2 m_position;
  glm::ivec2 m_size;
  using WidgetArray = boost::multi_array<std::shared_ptr<Widget>, 2>;
  WidgetArray m_widgets{};
  std::tuple<size_t, size_t> m_selected{0, 0};
  glm::ivec2 m_separation;
  std::vector<int> m_columnSizes{};
  std::vector<int> m_rowSizes{};
};
} // namespace ui::widgets
