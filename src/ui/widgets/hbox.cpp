#include "hbox.h"

#include "engine/presenter.h"
#include "ui/label.h"

namespace ui::widgets
{
HBox::HBox(const glm::ivec2& position, const glm::ivec2& size)
    : m_position{position}
    , m_size{size}
{
}

HBox::~HBox() = default;

void HBox::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  int x = m_position.x;
  for(const auto& widget : m_widgets)
  {
    widget->setPosition({x, m_position.y});
    widget->setSize({widget->getSize().x, m_size.y});
    widget->draw(ui, presenter);

    x += widget->getSize().x + ui::OutlineBorderWidth;
  }
}

size_t HBox::append(const gsl::not_null<std::shared_ptr<Widget>>& widget)
{
  m_widgets.emplace_back(widget);
  return m_widgets.size() - 1;
}

void HBox::setPosition(const glm::ivec2& position)
{
  m_position = position;
}

void HBox::update(bool hasFocus)
{
  for(size_t i = 0; i < m_widgets.size(); ++i)
    m_widgets[i]->update(hasFocus && m_selected == i);
}

glm::ivec2 HBox::getSize() const
{
  return m_size;
}

void HBox::setSize(const glm::ivec2& size)
{
  m_size = size;
}

void HBox::fitToContent()
{
  int x = 0;
  int maxHeight = 0;
  int maxWidth = 0;
  for(const auto& widget : m_widgets)
  {
    widget->fitToContent();

    x += widget->getSize().x;
    maxHeight = std::max(maxHeight, widget->getSize().y);
    maxWidth = std::max(maxWidth, x);

    x += ui::OutlineBorderWidth;
  }

  m_size = {maxWidth, maxHeight};
}
} // namespace ui::widgets
