#include "gridbox.h"

#include "engine/presenter.h"
#include "ui/label.h"

namespace ui::widgets
{
GridBox::GridBox(const glm::ivec2& position, const glm::ivec2& size, const glm::ivec2& separation)
    : m_position{position}
    , m_size{size}
    , m_separation{separation}
{
}

GridBox::~GridBox() = default;

void GridBox::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  int xPos = m_position.x;
  for(size_t x = 0; x < m_widgets.shape()[0]; ++x)
  {
    int yPos = m_position.y;
    for(size_t y = 0; y < m_widgets.shape()[1]; ++y)
    {
      const auto& widget = m_widgets[x][y];
      if(widget == nullptr)
        continue;

      widget->setPosition({xPos, yPos});
      widget->setSize({m_columnSizes[x], m_rowSizes[y]});
      widget->draw(ui, presenter);

      yPos += m_rowSizes[y] + m_separation.y;
    }
    xPos += m_columnSizes[x] + m_separation.x;
  }
}

void GridBox::set(size_t x, size_t y, const gsl::not_null<std::shared_ptr<Widget>>& widget)
{
  if(x >= m_widgets.shape()[0] || y >= m_widgets.shape()[1])
    BOOST_THROW_EXCEPTION(std::out_of_range("cell coordinates out of range"));
  m_widgets[x][y] = widget;
}

void GridBox::setPosition(const glm::ivec2& position)
{
  m_position = position;
}

void GridBox::update(bool hasFocus)
{
  for(size_t x = 0; x < m_widgets.shape()[0]; ++x)
  {
    for(size_t y = 0; y < m_widgets.shape()[0]; ++y)
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
  int totalWidth = 0;
  for(size_t x = 0; x < m_widgets.shape()[0]; ++x)
  {
    int maxWidth = 0;
    for(size_t y = 0; y < m_widgets.shape()[1]; ++y)
    {
      const auto& widget = m_widgets[x][y];
      if(widget == nullptr)
        continue;

      widget->fitToContent();
      maxWidth = std::max(widget->getSize().x, maxWidth);
    }
    m_columnSizes[x] = maxWidth;
    totalWidth += maxWidth + m_separation.x;
  }
  if(totalWidth > 0)
    totalWidth -= m_separation.x;

  int totalHeight = 0;
  for(size_t y = 0; y < m_widgets.shape()[1]; ++y)
  {
    int maxHeight = 0;
    for(size_t x = 0; x < m_widgets.shape()[0]; ++x)
    {
      const auto& widget = m_widgets[x][y];
      if(widget == nullptr)
        continue;

      maxHeight = std::max(widget->getSize().y, maxHeight);
    }
    m_rowSizes[y] = maxHeight;
    totalHeight += maxHeight + m_separation.y;
  }
  if(totalHeight > 0)
    totalHeight -= m_separation.y;

  m_size = {totalWidth, totalHeight};
}
} // namespace ui::widgets
