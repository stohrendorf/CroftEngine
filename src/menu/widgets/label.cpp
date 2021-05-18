#include "label.h"

#include "engine/presenter.h"
#include "menu/util.h"
#include "ui/core.h"
#include "ui/label.h"

namespace menu::widgets
{
Label::Label(const glm::ivec2& position, const std::string& label, int width)
    : m_label{std::make_unique<ui::Label>(position, label)}
{
  m_label->bgndSize = glm::ivec2{width, 0};
}

Label::~Label() = default;

glm::ivec2 Label::getPosition() const
{
  return m_label->pos;
}
glm::ivec2 Label::getSize() const
{
  return {m_label->bgndSize.x, ui::FontHeight};
}
void Label::setPosition(const glm::ivec2& position)
{
  m_label->pos = position;
}

void Label::update(bool hasFocus)
{
  if(hasFocus)
    markSelected(*m_label);
  else
    resetMarks(*m_label);
}

void Label::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  m_label->draw(ui, presenter.getTrFont(), presenter.getViewport());
}
} // namespace menu::widgets
