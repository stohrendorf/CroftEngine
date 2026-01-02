#include "listbox.h"

#include "ui/boxgouraud.h"
#include "ui/core.h"
#include "widget.h"

#include <algorithm>
#include <boost/assert.hpp>
#include <cstddef>
#include <gl/pixel.h>
#include <glm/fwd.hpp>
#include <gsl-lite/gsl-lite.hpp>
#include <gslu.h>
#include <memory>

namespace ui::widgets
{
ListBox::ListBox(const size_t pageSize)
    : m_pageSize{pageSize}
{
}

ListBox::~ListBox() = default;

void ListBox::draw(Ui& ui, const engine::Presenter& presenter) const
{
  size_t first = 0;
  size_t last = m_widgets.size();

  if(m_pageSize != 0)
  {
    const auto page = getCurrentPage();
    first = page * m_pageSize;
    last = std::min(first + m_pageSize, m_widgets.size());
  }
  gsl_Assert(first < last);

  static constexpr auto outerCorner = gl::SRGBA8{0, 0, 0, 0};
  static constexpr auto center = gl::SRGBA8{128, 64, 64, DefaultBackgroundAlpha};
  static constexpr auto innerCorner = gl::SRGBA8{128, 64, 0, DefaultBackgroundAlpha};
  static constexpr auto innerCenter = gl::SRGBA8{255, 255, 255, 255};
  static constexpr auto scrollIndicatorBottom = BackgroundGouraud{
    BoxGouraud{
      outerCorner,
      outerCorner,
      center,
      outerCorner,
    },
    BoxGouraud{
      outerCorner,
      outerCorner,
      outerCorner,
      center,
    },
    BoxGouraud{
      center,
      outerCorner,
      innerCorner,
      innerCenter,
    },
    BoxGouraud{
      outerCorner,
      center,
      innerCenter,
      innerCorner,
    },
  };

  if(first != 0)
  {
    scrollIndicatorBottom.mirroredY().draw(
      ui, m_position - glm::ivec2{0, FontHeight}, glm::ivec2{m_size.x, FontHeight});
  }
  if(last != m_widgets.size())
  {
    scrollIndicatorBottom.draw(
      ui, m_position + glm::ivec2{0, m_size.y - FontHeight - 2 * OutlineBorderWidth}, glm::ivec2{m_size.x, FontHeight});
  }

  int y = m_position.y;
  for(size_t i = first; i < last; ++i)
  {
    const auto& widget = m_widgets.at(i);
    widget->setPosition({m_position.x, y});
    widget->setSize({m_size.x, widget->getSize().y});
    widget->draw(ui, presenter);

    y += widget->getSize().y + OutlineBorderWidth;
  }
}

size_t ListBox::append(const gslu::nn_shared<Widget>& widget)
{
  m_widgets.emplace_back(widget);
  return m_widgets.size() - 1;
}

void ListBox::setPosition(const glm::ivec2& position)
{
  m_position = position;
}

void ListBox::update(const bool hasFocus)
{
  for(size_t i = 0; i < m_widgets.size(); ++i)
    m_widgets[i]->update(hasFocus && m_selected == i);
}

glm::ivec2 ListBox::getSize() const
{
  return m_size;
}

void ListBox::setSize(const glm::ivec2& size)
{
  m_size = size;
}

void ListBox::fitToContent()
{
  int y = 0;
  int maxHeight = 0;
  int maxWidth = 0;
  for(size_t i = 0; i < m_widgets.size(); ++i)
  {
    if(m_pageSize != 0 && i % m_pageSize == 0)
      y = 0;

    const auto& widget = m_widgets[i];

    y += widget->getSize().y;
    maxHeight = std::max(maxHeight, y);
    maxWidth = std::max(maxWidth, widget->getSize().x);

    y += OutlineBorderWidth;
  }

  m_size = {maxWidth, maxHeight};
}

size_t ListBox::getCurrentPage() const
{
  BOOST_ASSERT(m_pageSize > 0);
  return m_selected / m_pageSize;
}

size_t ListBox::getTotalPages() const
{
  BOOST_ASSERT(m_pageSize > 0);
  return (m_widgets.size() + m_pageSize - 1) / m_pageSize;
}
} // namespace ui::widgets