#include "textbox.h"

#include "engine/presenter.h"
#include "ui/core.h"
#include "ui/text.h"
#include "ui/ui.h"

#include <algorithm>
#include <gl/pixel.h>
#include <gsl/gsl-lite.hpp>
#include <numeric>

namespace ui::widgets
{
TextBox::TextBox(const std::string& text, int maxWidth, TextBox::Alignment alignment)
    : m_text{}
    , m_lines{}
    , m_size{maxWidth, ui::FontHeight}
    , m_alignment{alignment}
{
  setText(text);
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
  // re-flow the text
  auto text = std::move(m_text);
  setText(text);
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

  int y = ui::FontHeight / 2;
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

    line->draw(ui, presenter.getTrFont(), {m_position.x + x + ui::FontHeight / 2, m_position.y + y});
    y += ui::FontHeight;
  }
}

void TextBox::fitToContent()
{
  auto maxWidth = std::accumulate(m_lines.begin(),
                                  m_lines.end(),
                                  0,
                                  [](int width, auto& line)
                                  {
                                    return std::max(line->getWidth(), width);
                                  });

  m_size = {maxWidth + ui::FontHeight, ui::FontHeight * m_lines.size() + ui::FontHeight};
}

void TextBox::setText(const std::string& text)
{
  if(std::exchange(m_text, text) == text)
    return;

  auto broken = breakLines(text, m_size.x - ui::FontHeight);
  m_lines.clear();
  std::transform(broken.begin(),
                 broken.end(),
                 std::back_inserter(m_lines),
                 [](const auto& line)
                 {
                   return std::make_unique<ui::Text>(line);
                 });
}
} // namespace ui::widgets
