#pragma once

#include "ui/core.h"
#include "widget.h"

#include <algorithm>
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4458)
#  pragma warning(disable : 4459)
#endif
#include <boost/multi_array.hpp>
#include <boost/multi_array/base.hpp>
#include <boost/multi_array/extent_gen.hpp>
#include <boost/multi_array/multi_array_ref.hpp>
#include <boost/multi_array/subarray.hpp>
#ifdef _MSC_VER
#  pragma warning(pop)
#endif
#include <cstddef>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <map>
#include <memory>
#include <tuple>
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
class GridBox : public Widget
{
public:
  explicit GridBox(const glm::ivec2& separation = glm::ivec2{ui::OutlineBorderWidth, ui::OutlineBorderWidth});
  ~GridBox() override;
  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override;

  void set(size_t x, size_t y, const std::shared_ptr<Widget>& widget);

  [[nodiscard]] const auto& getSelected() const noexcept
  {
    return m_selected;
  }

  void setSelected(const std::pair<size_t, size_t>& selected) noexcept
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
    return next<0>();
  }

  template<size_t N>
  bool prev() noexcept
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
    m_widgets.resize(boost::extents[gsl::narrow<WidgetArray::index>(x)][gsl::narrow<WidgetArray::index>(y)]);
    m_alignRight.resize(x, false);
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

  [[nodiscard]] const auto& getColumnSizes() const noexcept
  {
    return m_columnSizes;
  }

  [[nodiscard]] const auto& getWidget(size_t x, size_t y) const
  {
    auto [sx, sy] = getExtents();
    gsl_Assert(x < sx && y < sy);
    return m_widgets[gsl::narrow<WidgetArray::index>(x)][gsl::narrow<WidgetArray::index>(y)];
  }

  [[nodiscard]] const auto& getSelectedWidget() const
  {
    auto [x, y] = m_selected;
    return getWidget(x, y);
  }

  void setAlignRight(size_t column, bool right)
  {
    m_alignRight.at(column) = right;
  }

private:
  void recalculateTotalSize();

  glm::ivec2 m_position{0, 0};
  glm::ivec2 m_size{0, 0};
  using WidgetArray = boost::multi_array<std::shared_ptr<Widget>, 2>;
  WidgetArray m_widgets{};
  std::tuple<size_t, size_t> m_selected{0, 0};
  glm::ivec2 m_separation;
  std::vector<bool> m_alignRight{};
  std::vector<int> m_columnSizes{};
  std::vector<int> m_rowSizes{};
};
} // namespace ui::widgets
