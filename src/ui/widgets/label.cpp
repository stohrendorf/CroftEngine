#include "label.h"

#include "engine/presenter.h"
#include "ui/core.h"
#include "ui/label.h"
#include "ui/ui.h"
#include "ui/util.h"

namespace ui::widgets
{
Label::Label(const glm::ivec2& position, const std::string& text, int width)
    : m_text{std::make_unique<ui::Text>(text)}
    , m_position{position}
    , m_size{width, ui::FontHeight}
{
}

Label::~Label() = default;

glm::ivec2 Label::getPosition() const
{
  return m_position;
}

glm::ivec2 Label::getSize() const
{
  return m_size;
}

void Label::setPosition(const glm::ivec2& position)
{
  m_position = position;
}

void Label::setSize(const glm::ivec2& size)
{
  m_size = size;
}

void Label::update(bool hasFocus)
{
  constexpr int FadeSpeed = 30;
  const auto delta = hasFocus ? FadeSpeed : -FadeSpeed;
  m_alpha = gsl::narrow_cast<uint8_t>(std::clamp(m_alpha + delta, 0, 255));
}

void Label::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  if(m_alpha != 0)
  {
    const auto bgPos = m_position - glm::ivec2{OutlineBorderWidth, FontHeight - OutlineBorderWidth / 2};
    const auto bgSize = m_size + glm::ivec2{2 * OutlineBorderWidth, 0};
    ui.drawBox(bgPos, bgSize, gl::SRGBA8{0, 0, 0, 224 * m_alpha / 255});
    ui.drawOutlineBox(bgPos, bgSize, m_alpha);
  }
  m_text->draw(ui, presenter.getTrFont(), m_position);
}

void Label::fitToContent()
{
  m_size = {m_text->getWidth(), ui::FontHeight};
}

void Label::setText(const std::string& text)
{
  m_text = std::make_unique<ui::Text>(text);
}
} // namespace ui::widgets
