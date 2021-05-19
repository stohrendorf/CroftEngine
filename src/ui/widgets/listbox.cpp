#include "listbox.h"

#include "engine/presenter.h"
#include "ui/util.h"

namespace ui::widgets
{
ListBox::ListBox(const glm::ivec2& position, const glm::ivec2& size, size_t pageSize)
    : m_position{position}
    , m_size{size}
    , m_pageSize{pageSize}
{
}

ListBox::~ListBox() = default;

void ListBox::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  const auto page = m_selected / m_pageSize;
  const auto first = page * m_pageSize;
  const auto last = std::min(first + m_pageSize, m_widgets.size());
  Expects(first < last);

  int y = m_position.y;
  for(size_t i = first; i < last; ++i)
  {
    const auto& widget = m_widgets.at(i);
    widget->setPosition({m_position.x, y});
    widget->setSize({m_size.x, widget->getSize().y});
    widget->draw(ui, presenter);

    y += widget->getSize().y + ui::OutlineBorderWidth;
  }
}

size_t ListBox::addEntry(const std::shared_ptr<Widget>& widget)
{
  m_widgets.emplace_back(widget);
  return m_widgets.size() - 1;
}

void ListBox::setPosition(const glm::ivec2& position)
{
  m_position = position;
}

void ListBox::update(bool hasFocus)
{
  for(size_t i = 0; i < m_widgets.size(); ++i)
    m_widgets.at(i)->update(hasFocus && m_selected == i);
}

glm::ivec2 ListBox::getSize() const
{
  return m_size;
}

void ListBox::setSize(const glm::ivec2& size)
{
  m_size = size;
}
} // namespace ui::widgets
