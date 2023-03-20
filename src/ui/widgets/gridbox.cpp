#include "gridbox.h"

#include "widget.h"

#include <boost/throw_exception.hpp>
#include <functional>
#include <glm/fwd.hpp>
#include <numeric>
#include <stdexcept>

namespace ui::widgets
{
GridBox::GridBox(const glm::ivec2& separation)
    : m_separation{separation}
{
}

GridBox::~GridBox() = default;

void GridBox::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  gsl_Assert(m_columnSizes.size() == m_widgets.shape()[0]);
  gsl_Assert(m_alignRight.size() == m_widgets.shape()[0]);
  gsl_Assert(m_rowSizes.size() == m_widgets.shape()[1]);

  int xPos = m_position.x;
  for(WidgetArray::index x = 0; x < m_widgets.shape()[0]; ++x)
  {
    int yPos = m_position.y;
    for(WidgetArray::index y = 0; y < m_widgets.shape()[1]; ++y)
    {
      const auto& widget = m_widgets[x][y];
      if(widget != nullptr)
      {
        int alignment = 0;
        if(m_alignRight[x])
          alignment = m_columnSizes[x] - widget->getSize().x;
        widget->setPosition({xPos + alignment, yPos});
        widget->draw(ui, presenter);
      }

      yPos += m_rowSizes[y] + m_separation.y;
    }
    xPos += m_columnSizes[x] + m_separation.x;
  }
}

void GridBox::set(size_t x, size_t y, const std::shared_ptr<Widget>& widget)
{
  if(x >= m_widgets.shape()[0] || y >= m_widgets.shape()[1])
    BOOST_THROW_EXCEPTION(std::out_of_range("cell coordinates out of range"));
  m_widgets[gsl::narrow<WidgetArray::index>(x)][gsl::narrow<WidgetArray::index>(y)] = widget;
}

void GridBox::setPosition(const glm::ivec2& position)
{
  m_position = position;
}

void GridBox::update(bool hasFocus)
{
  for(WidgetArray::index x = 0; x < m_widgets.shape()[0]; ++x)
  {
    for(WidgetArray::index y = 0; y < m_widgets.shape()[1]; ++y)
    {
      const auto& widget = m_widgets[x][y];
      if(widget == nullptr)
        continue;

      widget->update(hasFocus && m_selected == std::tuple{x, y});
    }
  }
}

glm::ivec2 GridBox::getSize() const
{
  return m_size;
}

void GridBox::setSize(const glm::ivec2& size)
{
  m_size = size;
}

void GridBox::fitToContent()
{
  gsl_Assert(m_columnSizes.size() == m_widgets.shape()[0]);
  gsl_Assert(m_alignRight.size() == m_widgets.shape()[0]);
  gsl_Assert(m_rowSizes.size() == m_widgets.shape()[1]);

  for(WidgetArray::index x = 0; x < m_widgets.shape()[0]; ++x)
  {
    int maxWidth = 0;
    for(WidgetArray::index y = 0; y < m_widgets.shape()[1]; ++y)
    {
      const auto& widget = m_widgets[x][y];
      if(widget == nullptr)
        continue;

      maxWidth = std::max(widget->getSize().x, maxWidth);
    }
    m_columnSizes[x] = maxWidth;
  }

  for(WidgetArray::index y = 0; y < m_widgets.shape()[1]; ++y)
  {
    int maxHeight = 0;
    for(WidgetArray::index x = 0; x < m_widgets.shape()[0]; ++x)
    {
      const auto& widget = m_widgets[x][y];
      if(widget == nullptr)
        continue;

      maxHeight = std::max(widget->getSize().y, maxHeight);
    }
    m_rowSizes[y] = maxHeight;
  }

  recalculateTotalSize();
}

void GridBox::recalculateTotalSize()
{
  auto totalWidth = std::accumulate(m_columnSizes.begin(), m_columnSizes.end(), 0, std::plus<>{});
  if(!m_columnSizes.empty())
    totalWidth += gsl::narrow<int>(m_columnSizes.size() - 1) * m_separation.x;

  auto totalHeight = std::accumulate(m_rowSizes.begin(), m_rowSizes.end(), 0, std::plus<>{});
  if(!m_rowSizes.empty())
    totalHeight += gsl::narrow<int>(m_rowSizes.size() - 1) * m_separation.y;

  m_size = {totalWidth, totalHeight};
}
} // namespace ui::widgets
