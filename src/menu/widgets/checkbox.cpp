#include "checkbox.h"

#include "engine/presenter.h"
#include "menu/util.h"
#include "ui/core.h"
#include "ui/label.h"
#include "ui/ui.h"

namespace menu::widgets
{
constexpr int TextOffset = 20;

Checkbox::Checkbox(const glm::ivec2& position, const std::string& label, int width)
    : m_label{std::make_unique<ui::Label>(position, label)}
{
  m_label->bgndSize = glm::ivec2{width - TextOffset, 0};
}

Checkbox::~Checkbox() = default;

void Checkbox::draw(ui::Ui& ui, const engine::Presenter& presenter)
{
  m_label->draw(ui, presenter.getTrFont(), presenter.getViewport());

  static constexpr int BoxSize = ui::FontHeight - 2 * ui::OutlineBorderWidth;
  static constexpr int InnerOffset = 3;
  static constexpr int InnerSize = BoxSize - 2 * InnerOffset + 1;
  ui.drawOutlineBox(m_label->pos + glm::ivec2{ui::OutlineBorderWidth - TextOffset, 3 - ui::FontHeight},
                    {BoxSize, BoxSize});
  if(m_checked)
  {
    ui.drawBox(m_label->pos
                 + glm::ivec2{ui::OutlineBorderWidth + InnerOffset - TextOffset, 3 + InnerOffset - ui::FontHeight},
               {InnerSize, InnerSize},
               15);
  }
}

void Checkbox::update(bool selected)
{
  if(selected)
    markSelected(*m_label);
  else
    resetMarks(*m_label);
}

void Checkbox::setPosition(const glm::ivec2& position)
{
  m_label->pos = position + glm::ivec2{TextOffset, 0};
}
} // namespace menu::widgets
