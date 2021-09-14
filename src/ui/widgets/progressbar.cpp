#include "progressbar.h"

#include "ui/core.h"
#include "ui/ui.h"

#include <algorithm>
#include <gsl/gsl-lite.hpp>

namespace ui::widgets
{
ProgressBar::~ProgressBar() = default;

void ProgressBar::draw(ui::Ui& ui, const engine::Presenter& /*presenter*/) const
{
  static constexpr int InnerOffset = ui::OutlineBorderWidth + 1;

  auto innerSize
    = gsl::narrow_cast<int>(gsl::narrow_cast<float>(getSize().x - 2 * InnerOffset) * std::clamp(m_value, 0.0f, 1.0f));
  ui.drawOutlineBox(m_position - glm::ivec2{0, ui::FontHeight - 1}, getSize());
  ui.drawBox(m_position + glm::ivec2{InnerOffset, InnerOffset - ui::FontHeight + 1},
             glm::ivec2{innerSize + 1, getSize().y - 2 * InnerOffset + 1},
             15);
}

void ProgressBar::update(bool /*hasFocus*/)
{
}

void ProgressBar::setPosition(const glm::ivec2& position)
{
  m_position = position;
}

glm::ivec2 ProgressBar::getPosition() const
{
  return m_position;
}

glm::ivec2 ProgressBar::getSize() const
{
  return m_size;
}

void ProgressBar::setSize(const glm::ivec2& size)
{
  m_size = size;
}

void ProgressBar::fitToContent()
{
}
} // namespace ui::widgets
