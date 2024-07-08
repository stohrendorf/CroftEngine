#include "groupbox.h"

#include "engine/presenter.h"
#include "ui/boxgouraud.h"
#include "ui/core.h"
#include "ui/text.h"
#include "ui/ui.h"
#include "ui/widgets/widget.h"

#include <gl/pixel.h>
#include <glm/fwd.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <string>
#include <utility>

namespace ui::widgets
{
constexpr int TitleOffset = 10;
constexpr int InnerMargin = 10;
constexpr int WidgetOffsetTop = ui::FontHeight / 2 + InnerMargin + ui::OutlineBorderWidth;
constexpr int TotalVerticalMargin = WidgetOffsetTop + InnerMargin + 2 * ui::OutlineBorderWidth;

GroupBox::GroupBox(const std::string& title, gslu::nn_shared<Widget> widget)
    : m_title{std::make_unique<ui::Text>(title)}
    , m_widget{std::move(widget)}
{
  m_widget->setPosition(m_position + glm::ivec2{InnerMargin, WidgetOffsetTop});
}

GroupBox::~GroupBox() = default;

void GroupBox::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  const auto bgPos = m_position - glm::ivec2{OutlineBorderWidth, FontHeight / 2 - 1};
  const auto bgSize = m_size - glm::ivec2{-2 * OutlineBorderWidth, ui::FontHeight / 2 + 2 * ui::OutlineBorderWidth};

  UiBackdrop.draw(ui, bgPos, bgSize);

  m_widget->setPosition(m_position + glm::ivec2{InnerMargin, WidgetOffsetTop});
  m_widget->setSize({m_size.x - 2 * InnerMargin, m_size.y - TotalVerticalMargin});
  m_widget->draw(ui, presenter);

  ui.drawOutlineBox(bgPos, bgSize);

  const auto bgPosTitle = m_position - glm::ivec2{OutlineBorderWidth - TitleOffset, FontHeight - 1};
  const auto bgSizeTitle = glm::ivec2{m_title->getWidth() + 2 * OutlineBorderWidth, ui::FontHeight};
  ui.drawBox(bgPosTitle, bgSizeTitle, gl::SRGBA8{0, 0, 0, 255});
  ui.drawOutlineBox(bgPosTitle, bgSizeTitle);
  m_title->draw(ui, presenter.getTrFont(), m_position + glm::ivec2{TitleOffset, 0});
}

void GroupBox::setPosition(const glm::ivec2& position)
{
  m_position = position;
}

void GroupBox::update(bool hasFocus)
{
  m_widget->update(hasFocus);
}

glm::ivec2 GroupBox::getSize() const
{
  return m_size;
}

void GroupBox::setSize(const glm::ivec2& size)
{
  m_size = size;
}

void GroupBox::fitToContent()
{
  m_size = m_widget->getSize() + glm::ivec2{2 * InnerMargin, TotalVerticalMargin};
}

void GroupBox::setTitle(const std::string& title)
{
  m_title = gsl::make_unique<ui::Text>(title);
}
} // namespace ui::widgets
