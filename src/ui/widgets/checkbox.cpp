#include "checkbox.h"

#include "label.h"
#include "ui/core.h"
#include "ui/ui.h"
#include "widget.h"

#include <glm/fwd.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <utility>

namespace ui::widgets
{
constexpr int ContentOffset = 20;

Checkbox::Checkbox(const std::string& label)
    : m_content{gsl::make_shared<Label>(label)}
{
}

Checkbox::Checkbox(gslu::nn_shared<Widget> content)
    : m_content{std::move(content)}
{
}

Checkbox::~Checkbox() = default;

void Checkbox::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  m_content->setSize({m_size.x - ContentOffset, m_size.y});
  m_content->draw(ui, presenter);

  static constexpr int BoxSize = ui::FontHeight - 2 * ui::OutlineBorderWidth;
  static constexpr int InnerOffset = 3;
  static constexpr int InnerSize = BoxSize - 2 * InnerOffset + 1;
  ui.drawOutlineBox(m_content->getPosition() + glm::ivec2{ui::OutlineBorderWidth - ContentOffset, 3 - ui::FontHeight},
                    {BoxSize, BoxSize});
  if(m_checked)
  {
    ui.drawBox(m_content->getPosition()
                 + glm::ivec2{ui::OutlineBorderWidth + InnerOffset - ContentOffset, 3 + InnerOffset - ui::FontHeight},
               {InnerSize, InnerSize},
               15);
  }
}

void Checkbox::update(bool hasFocus)
{
  m_content->update(hasFocus);
}

void Checkbox::setPosition(const glm::ivec2& position)
{
  m_content->setPosition(position + glm::ivec2{ContentOffset, 0});
}

glm::ivec2 Checkbox::getPosition() const
{
  return m_content->getPosition() - glm::ivec2{ContentOffset, 0};
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
  m_content->fitToContent();
  m_size = m_content->getSize() + glm::ivec2{ContentOffset, 0};
}
} // namespace ui::widgets
