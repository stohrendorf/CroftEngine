#include "label.h"

#include "engine/presenter.h"
#include "ui/core.h"
#include "ui/text.h"
#include "ui/ui.h"

#include <algorithm>
#include <cstdint>
#include <gl/pixel.h>
#include <gsl-lite/gsl-lite.hpp>
#include <memory>
#include <string>

namespace ui::widgets
{
Label::Label(const std::string& text, const Alignment alignment)
    : m_text{std::make_unique<Text>(text)}
    , m_size{0, FontHeight}
    , m_alignment{alignment}
{
  fitToContent();
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

void Label::update(const bool hasFocus)
{
  constexpr int FadeSpeed = 30;
  const auto delta = hasFocus ? FadeSpeed : -FadeSpeed;
  m_selectionAlpha = gsl_lite::narrow_cast<uint8_t>(std::clamp(m_selectionAlpha + delta, 0, 255));
}

void Label::draw(Ui& ui, const engine::Presenter& presenter) const
{
  if(m_selectionAlpha != 0)
  {
    const auto bgPos = m_position - glm::ivec2{OutlineBorderWidth, FontHeight - OutlineBorderWidth / 2};
    const auto bgSize = m_size + glm::ivec2{2 * OutlineBorderWidth, 0};
    ui.drawBox(
      bgPos,
      bgSize,
      gl::SRGBA8{
        0, 0, 0, static_cast<uint8_t>(static_cast<uint16_t>(DefaultBackgroundAlpha) * m_selectionAlpha / 255)});
    ui.drawOutlineBox(bgPos, bgSize, m_selectionAlpha);
  }

  auto x = 0;
  switch(m_alignment)
  {
  case Alignment::Left:
    break;
  case Alignment::Center:
    x = (m_size.x - m_text->getWidth()) / 2;
    break;
  case Alignment::Right:
    x = m_size.x - m_text->getWidth();
    break;
  }

  m_text->draw(ui, presenter.getTrFont(), {m_position.x + x, m_position.y});
}

void Label::fitToContent()
{
  m_size = {m_text->getWidth(), FontHeight};
}

void Label::setText(const std::string& text)
{
  m_text = std::make_unique<Text>(text);
}
} // namespace ui::widgets