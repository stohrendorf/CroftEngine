#include "listbox.h"

#include "checkbox.h"
#include "engine/presenter.h"
#include "menu/util.h"
#include "ui/label.h"
#include "ui/ui.h"

namespace menu::widgets
{
ListBox::ListBox(size_t pageSize, int width, const glm::ivec2& position)
    : m_pageSize{pageSize}
    , m_width{width}
    , m_position{position}
{
}

ListBox::~ListBox() = default;

void ListBox::draw(ui::Ui& ui, const engine::Presenter& presenter)
{
  const auto page = m_selected / m_pageSize;
  const auto first = page * m_pageSize;
  const auto last = std::min(first + m_pageSize, m_checkboxes.size());
  Expects(first < last);

  for(size_t i = 0; i < m_checkboxes.size(); ++i)
    m_checkboxes.at(i)->update(m_selected == i);
  for(size_t i = first; i < last; ++i)
    m_checkboxes.at(i)->draw(ui, presenter);
}

size_t ListBox::addEntry(const std::string& label)
{
  auto checkbox = std::make_unique<Checkbox>(m_position, label, m_width);
  m_checkboxes.emplace_back(std::move(checkbox));
  return m_checkboxes.size() - 1;
}

void ListBox::setChecked(size_t idx, bool checked)
{
  m_checkboxes.at(idx)->setChecked(checked);
}

void ListBox::setPosition(const glm::ivec2& position)
{
  m_position = position;
  for(size_t i = 0; i < m_checkboxes.size(); ++i)
  {
    const auto inPage = i % m_pageSize;
    m_checkboxes[i]->setPosition({m_position.x, m_position.y + inPage * EntryHeight});
  }
}
} // namespace menu::widgets
