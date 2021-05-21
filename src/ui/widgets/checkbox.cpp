#include "checkbox.h"

#include "engine/presenter.h"
#include "label.h"
#include "ui/core.h"
#include "ui/ui.h"
#include "ui/util.h"

namespace ui::widgets
{
constexpr int TextOffset = 20;

Checkbox::Checkbox(const glm::ivec2& position, const std::string& label, int width)
    : m_size{width, ui::FontHeight}
    , m_label{std::make_unique<Label>(position, label)}
{
}

Checkbox::~Checkbox() = default;

void Checkbox::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  m_label->setSize({m_size.x - TextOffset, m_size.y});
  m_label->draw(ui, presenter);

  static constexpr int BoxSize = ui::FontHeight - 2 * ui::OutlineBorderWidth;
  static constexpr int InnerOffset = 3;
  static constexpr int InnerSize = BoxSize - 2 * InnerOffset + 1;
  ui.drawOutlineBox(m_label->getPosition() + glm::ivec2{ui::OutlineBorderWidth - TextOffset, 3 - ui::FontHeight},
                    {BoxSize, BoxSize});
  if(m_checked)
  {
    ui.drawBox(m_label->getPosition()
                 + glm::ivec2{ui::OutlineBorderWidth + InnerOffset - TextOffset, 3 + InnerOffset - ui::FontHeight},
               {InnerSize, InnerSize},
               15);
  }
}

void Checkbox::update(bool hasFocus)
{
  m_label->update(hasFocus);
}

void Checkbox::setPosition(const glm::ivec2& position)
{
  m_label->setPosition(position + glm::ivec2{TextOffset, 0});
}

glm::ivec2 Checkbox::getPosition() const
{
  return m_label->getPosition() - glm::ivec2{TextOffset, 0};
}

glm::ivec2 Checkbox::getSize() const
{
  return m_size;
}

void Checkbox::setSize(const glm::ivec2& size)
{
  m_size = size;
}

void Checkbox::fitToContent()
{
  m_label->fitToContent();
  m_size = m_label->getSize() + glm::ivec2{TextOffset, 0};
}

void Checkbox::setLabel(const std::string& label)
{
  m_label->setText(label);
}
} // namespace ui::widgets
