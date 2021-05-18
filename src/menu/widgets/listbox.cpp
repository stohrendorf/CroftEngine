#include "listbox.h"

#include "engine/presenter.h"
#include "menu/util.h"

namespace menu::widgets
{
ListBox::ListBox(size_t pageSize, int width, const glm::ivec2& position)
    : m_pageSize{pageSize}
    , m_width{width}
    , m_position{position}
{
}

ListBox::~ListBox() = default;

void ListBox::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  const auto page = m_selected / m_pageSize;
  const auto first = page * m_pageSize;
  const auto last = std::min(first + m_pageSize, m_widgets.size());
  Expects(first < last);

  for(size_t i = first; i < last; ++i)
    m_widgets.at(i)->draw(ui, presenter);
}

size_t ListBox::addEntry(const std::shared_ptr<Widget>& widget)
{
  m_widgets.emplace_back(widget);
  return m_widgets.size() - 1;
}

void ListBox::setPosition(const glm::ivec2& position)
{
  m_position = position;
  int y = 0;
  for(size_t i = 0; i < m_widgets.size(); ++i)
  {
    const auto inPage = i % m_pageSize;
    if(inPage == 0)
      y = m_position.y;

    m_widgets[i]->setPosition({m_position.x, y});
    y += m_widgets[i]->getSize().y + ui::OutlineBorderWidth;
  }
}

void ListBox::update(bool hasFocus)
{
  for(size_t i = 0; i < m_widgets.size(); ++i)
    m_widgets.at(i)->update(hasFocus && m_selected == i);
}

glm::ivec2 ListBox::getSize() const
{
  int maxY = 0;
  int y = 0;
  for(size_t i = 0; i < m_widgets.size(); ++i)
  {
    const auto inPage = i % m_pageSize;
    if(inPage == 0)
    {
      maxY = std::max(maxY, y);
      y = 0;
    }

    y += m_widgets[i]->getSize().y + ui::OutlineBorderWidth;
  }
  maxY = std::max(maxY, y);

  return glm::ivec2{m_width, std::max(0, maxY - ui::OutlineBorderWidth)};
}
} // namespace menu::widgets
