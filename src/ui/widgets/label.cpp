#include "label.h"

#include "engine/presenter.h"
#include "ui/core.h"
#include "ui/label.h"
#include "ui/util.h"

namespace ui::widgets
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

void Label::setSize(const glm::ivec2& size)
{
  m_label->bgndSize = size;
}

void Label::update(bool hasFocus)
{
  constexpr int FadeSpeed = 30;
  const auto delta = hasFocus ? FadeSpeed : -FadeSpeed;

  m_label->backgroundAlpha = gsl::narrow_cast<uint8_t>(std::clamp(m_label->backgroundAlpha + delta, 0, 224));
  m_label->outlineAlpha = gsl::narrow_cast<uint8_t>(std::clamp(m_label->outlineAlpha + delta, 0, 255));
  m_label->backgroundGouraudAlpha
    = gsl::narrow_cast<uint8_t>(std::clamp(m_label->backgroundGouraudAlpha + delta, 0, 255));
}

void Label::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  m_label->draw(ui, presenter.getTrFont(), presenter.getViewport());
}

void Label::fitToContent()
{
  m_label->bgndSize.x = m_label->calcWidth();
  m_label->bgndSize.y = ui::FontHeight;
}

void Label::setText(const std::string& text)
{
  m_label->text = text;
}
} // namespace ui::widgets
