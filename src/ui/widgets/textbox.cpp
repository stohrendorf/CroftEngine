#include "textbox.h"

#include "engine/presenter.h"
#include "ui/core.h"
#include "ui/text.h"
#include "ui/ui.h"

#include <algorithm>
#include <gl/pixel.h>
#include <gsl/gsl-lite.hpp>

namespace ui::widgets
{
TextBox::TextBox(const std::string& text, int maxWidth, TextBox::Alignment alignment)
    : m_lines{}
    , m_size{0, ui::FontHeight}
    , m_alignment{alignment}
{
  setText(text, maxWidth);
}

TextBox::~TextBox() = default;

glm::ivec2 TextBox::getPosition() const
{
  return m_position;
}

glm::ivec2 TextBox::getSize() const
{
  return m_size;
}

void TextBox::setPosition(const glm::ivec2& position)
{
  m_position = position;
}

void TextBox::setSize(const glm::ivec2& size)
{
  m_size = size;
}

void TextBox::update(bool /*hasFocus*/)
{
}

void TextBox::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  const auto bgPos = m_position - glm::ivec2{OutlineBorderWidth, FontHeight - OutlineBorderWidth / 2};
  const auto bgSize = m_size + glm::ivec2{2 * OutlineBorderWidth, 0};
  ui.drawBox(bgPos, bgSize, gl::SRGBA8{0, 0, 0, DefaultBackgroundAlpha});
  ui.drawOutlineBox(bgPos, bgSize, 255);

  int y = 0;
  for(const auto& line : m_lines)
  {
    auto x = 0;
    switch(m_alignment)
    {
    case Alignment::Left:
      break;
    case Alignment::Center:
      x = (m_size.x - line->getWidth()) / 2;
      break;
    case Alignment::Right:
      x = m_size.x - line->getWidth();
      break;
    }

    line->draw(ui, presenter.getTrFont(), {m_position.x + x, m_position.y + y});
    y += ui::FontHeight;
  }
}

void TextBox::fitToContent()
{
  int maxWidth = 0;
  for(const auto& line : m_lines)
  {
    maxWidth = std::max(line->getWidth(), maxWidth);
  }

  m_size = {maxWidth, ui::FontHeight * m_lines.size()};
}

void TextBox::setText(const std::string& text, int maxWidth)
{
  m_lines.clear();
  for(const auto& line : breakLines(text, maxWidth))
  {
    m_lines.emplace_back(std::make_unique<ui::Text>(line));
  }
}
} // namespace ui::widgets
