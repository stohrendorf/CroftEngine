#include "groupbox.h"

#include "engine/presenter.h"
#include "ui/label.h"
#include "ui/ui.h"
#include "ui/util.h"

#include <utility>

namespace ui::widgets
{
constexpr int TitleOffset = 10;
constexpr int InnerMargin = 10;
constexpr int WidgetOffsetTop = ui::FontHeight / 2 + InnerMargin + ui::OutlineBorderWidth;
constexpr int TotalVerticalMargin = WidgetOffsetTop + InnerMargin + 2 * ui::OutlineBorderWidth;

GroupBox::GroupBox(const glm::ivec2& position,
                   const glm::ivec2& size,
                   const std::string& title,
                   std::shared_ptr<Widget> widget)
    : m_position{position}
    , m_size{size}
    , m_title{createHeading(title, {0, 0}, {0, 0})}
    , m_widget{std::move(widget)}
{
  m_widget->setPosition(m_position + glm::ivec2{InnerMargin, WidgetOffsetTop});
  m_title->bgndSize.x = m_title->calcWidth();
}

GroupBox::~GroupBox() = default;

void GroupBox::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  createFrame(m_position + glm::ivec2{0, ui::FontHeight / 2},
              m_size - glm::ivec2{0, ui::FontHeight / 2 + 2 * ui::OutlineBorderWidth})
    ->draw(ui, presenter.getTrFont(), presenter.getViewport());
  m_title->pos = m_position + glm::ivec2{TitleOffset, 0};
  m_title->draw(ui, presenter.getTrFont(), presenter.getViewport());
  m_widget->setPosition(m_position + glm::ivec2{InnerMargin, WidgetOffsetTop});
  m_widget->setSize({m_size.x - 2 * InnerMargin, m_size.y - TotalVerticalMargin});
  m_widget->draw(ui, presenter);
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
  m_widget->fitToContent();
  m_size = m_widget->getSize() + glm::ivec2{2 * InnerMargin, TotalVerticalMargin};
}
} // namespace ui::widgets
