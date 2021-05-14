#include "checkbox.h"

#include "engine/presenter.h"
#include "menu/util.h"
#include "ui/label.h"
#include "ui/ui.h"

namespace menu::widgets
{
Checkbox::Checkbox(const glm::ivec2& pos, const std::string& label, const glm::ivec2& size)
    : m_label{std::make_unique<ui::Label>(pos, label)}
{
  m_label->alignX = ui::Label::Alignment::Center;
  m_label->alignY = ui::Label::Alignment::Bottom;
  m_label->bgndSize = size;
}

Checkbox::~Checkbox() = default;

void Checkbox::draw(ui::Ui& ui, const engine::Presenter& presenter, bool selected)
{
  resetMarks(*m_label);

  if(selected)
    markSelected(*m_label);

  m_label->draw(ui, presenter.getTrFont(), presenter.getViewport());

  if(m_checked)
  {
    ui.drawBox(m_label->getOrigin(presenter.getViewport()) + glm::ivec2{2, 2}, {6, 13}, 31);
    ui.drawBox(m_label->getOrigin(presenter.getViewport()) + glm::ivec2{3, 3}, {4, 11}, 15);
  }
}
} // namespace menu::widgets
