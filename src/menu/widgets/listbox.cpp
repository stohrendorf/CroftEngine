#include "listbox.h"

#include "checkbox.h"
#include "engine/presenter.h"
#include "menu/util.h"
#include "ui/label.h"
#include "ui/ui.h"

namespace menu::widgets
{
ListBox::ListBox(size_t pageSize, int pixelWidth, int bottomMargin)
    : m_pageSize{pageSize}
    , m_pixelWidth{pixelWidth}
    , m_bottomMargin{bottomMargin}
{
}

ListBox::~ListBox() = default;

void ListBox::draw(ui::Ui& ui, const engine::Presenter& presenter)
{
  const auto page = m_selected / m_pageSize;
  const auto first = page * m_pageSize;
  const auto last = std::min(first + m_pageSize, m_checkboxes.size());
  Expects(first < last);
  for(size_t i = first; i < last; ++i)
  {
    m_checkboxes.at(i)->draw(ui, presenter, m_selected == i);
  }
}

size_t ListBox::addEntry(const std::string& label)
{
  const auto line = m_checkboxes.size() % m_pageSize;
  auto checkbox
    = std::make_unique<Checkbox>(glm::ivec2{0, line * EntryHeight - (m_pageSize * EntryHeight + m_bottomMargin)},
                                 label,
                                 glm::ivec2{m_pixelWidth, 16});
  m_checkboxes.emplace_back(std::move(checkbox));
  return m_checkboxes.size() - 1;
}

void ListBox::setChecked(size_t idx, bool checked)
{
  m_checkboxes.at(idx)->setChecked(checked);
}
} // namespace menu::widgets
