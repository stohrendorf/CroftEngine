#include "listbox.h"

#include "engine/presenter.h"
#include "menu/util.h"
#include "ui/label.h"
#include "ui/ui.h"

namespace menu::widgets
{
ListBox::ListBox(size_t pageSize, int pixelWidth)
    : m_pageSize{pageSize}
    , m_pixelWidth{pixelWidth}
{
}

void ListBox::draw(ui::Ui& ui, const engine::Presenter& presenter)
{
  const auto page = m_selected / m_pageSize;
  const auto first = page * m_pageSize;
  const auto last = std::min(first + m_pageSize, m_labels.size());
  Expects(first < last);
  for(size_t i = first; i < last; ++i)
  {
    const auto& [lbl, checked] = m_labels.at(i);
    resetMarks(*lbl);

    if(m_selected == i)
      markSelected(*lbl);

    lbl->draw(ui, presenter.getTrFont(), presenter.getViewport());

    if(checked)
    {
      ui.drawBox(lbl->getOrigin(presenter.getViewport()) + glm::ivec2{2, 2}, {6, 13}, 31);
      ui.drawBox(lbl->getOrigin(presenter.getViewport()) + glm::ivec2{3, 3}, {4, 11}, 15);
    }
  }
}

size_t ListBox::addEntry(const std::string& label)
{
  const auto line = m_labels.size() % m_pageSize;
  auto lbl = std::make_unique<ui::Label>(
    glm::ivec2{0, line * EntryHeight - (m_pageSize * EntryHeight + 10 + BottomMargin)}, label);
  lbl->alignX = ui::Label::Alignment::Center;
  lbl->alignY = ui::Label::Alignment::Bottom;
  lbl->bgndSize = {m_pixelWidth - 12, 16};
  m_labels.emplace_back(std::move(lbl), false);
  return m_labels.size() - 1;
}

void ListBox::setActive(size_t idx, bool active)
{
  std::get<1>(m_labels.at(idx)) = active;
}
} // namespace menu::widgets
