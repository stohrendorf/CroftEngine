#include "messagebox.h"

#include "core/i18n.h"
#include "label.h"
#include "ui/core.h"
#include "ui/ui.h"

#include <gl/pixel.h>

namespace ui::widgets
{
MessageBox::MessageBox(const std::string& label)
    : m_question{std::make_shared<Label>(label)}
    , m_yes{std::make_shared<Label>(/* translators: TR charmap encoding */ _("Yes"))}
    , m_no{std::make_shared<Label>(/* translators: TR charmap encoding */ _("No"))}
{
  m_question->fitToContent();
  m_yes->fitToContent();
  m_no->fitToContent();
}

MessageBox::~MessageBox() = default;

void MessageBox::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  const auto center = m_position.x + getSize().x / 2;
  m_question->setPosition({m_position.x + ui::FontHeight, m_position.y + ui::FontHeight * 3 / 2});
  const auto answersWidth = m_yes->getSize().x + m_no->getSize().x + ui::FontHeight;
  const auto answersX = center - answersWidth / 2 - ui::FontHeight / 2;
  const auto answersY = m_position.y + ui::FontHeight * 7 / 2;
  m_yes->setPosition({answersX, answersY});
  m_no->setPosition({answersX + m_yes->getSize().x + ui::FontHeight, answersY});

  ui.drawBox(m_position, getSize(), gl::SRGBA8{0, 0, 0, 224});
  ui.drawOutlineBox(m_position, getSize());
  m_question->draw(ui, presenter);
  m_yes->draw(ui, presenter);
  m_no->draw(ui, presenter);
}

void MessageBox::update(bool hasFocus)
{
  m_yes->update(hasFocus && m_confirmed);
  m_no->update(hasFocus && !m_confirmed);
}

void MessageBox::setPosition(const glm::ivec2& position)
{
  m_position = position;
}

glm::ivec2 MessageBox::getPosition() const
{
  return m_position;
}

glm::ivec2 MessageBox::getSize() const
{
  return {m_question->getSize().x + 2 * ui::FontHeight, 4 * ui::FontHeight};
}

void MessageBox::setSize(const glm::ivec2& /*size*/)
{
}

void MessageBox::fitToContent()
{
}
} // namespace ui::widgets
