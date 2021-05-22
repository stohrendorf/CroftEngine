#include "listbox.h"

#include "engine/presenter.h"
#include "ui/label.h"

namespace ui::widgets
{
ListBox::ListBox(const glm::ivec2& position, const glm::ivec2& size, size_t pageSize)
    : m_position{position}
    , m_size{size}
    , m_pageSize{pageSize}
{
}

ListBox::~ListBox() = default;

void ListBox::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  size_t first = 0;
  size_t last = m_widgets.size();

  if(m_pageSize != 0)
  {
    const auto page = m_selected / m_pageSize;
    first = page * m_pageSize;
    last = std::min(first + m_pageSize, m_widgets.size());
  }
  Expects(first < last);

  static const auto outerCorner = gl::SRGBA8{0, 0, 0, 0};
  static const auto center = gl::SRGBA8{128, 64, 64, 128};
  static const auto innerCorner = gl::SRGBA8{128, 64, 0, 128};
  static const auto innerCenter = gl::SRGBA8{255, 255, 255, 255};
  static const auto scrollIndicatorBottom = ui::Label::BackgroundGouraud{
    ui::BoxGouraud{
      outerCorner,
      outerCorner,
      center,
      outerCorner,
    },
    ui::BoxGouraud{
      outerCorner,
      outerCorner,
      outerCorner,
      center,
    },
    ui::BoxGouraud{
      center,
      outerCorner,
      innerCorner,
      innerCenter,
    },
    ui::BoxGouraud{
      outerCorner,
      center,
      innerCenter,
      innerCorner,
    },
  };

  if(first != 0)
  {
    ui::Label indicator{m_position, ""};
    indicator.bgndSize = {m_size.x, 0};
    indicator.backgroundGouraudAlpha = 255;
    indicator.backgroundGouraud = scrollIndicatorBottom.mirroredY();
    indicator.draw(ui, presenter.getTrFont(), presenter.getViewport());
  }
  if(last != m_widgets.size())
  {
    ui::Label indicator{m_position + glm::ivec2{0, m_size.y - ui::FontHeight}, ""};
    indicator.bgndSize = {m_size.x, 0};
    indicator.backgroundGouraudAlpha = 255;
    indicator.backgroundGouraud = scrollIndicatorBottom;
    indicator.draw(ui, presenter.getTrFont(), presenter.getViewport());
  }

  int y = m_position.y;
  for(size_t i = first; i < last; ++i)
  {
    const auto& widget = m_widgets.at(i);
    widget->setPosition({m_position.x, y});
    widget->setSize({m_size.x, widget->getSize().y});
    widget->draw(ui, presenter);

    y += widget->getSize().y + ui::OutlineBorderWidth;
  }
}

size_t ListBox::addEntry(const std::shared_ptr<Widget>& widget)
{
  m_widgets.emplace_back(widget);
  return m_widgets.size() - 1;
}

void ListBox::setPosition(const glm::ivec2& position)
{
  m_position = position;
}

void ListBox::update(bool hasFocus)
{
  for(size_t i = 0; i < m_widgets.size(); ++i)
    m_widgets.at(i)->update(hasFocus && m_selected == i);
}

glm::ivec2 ListBox::getSize() const
{
  return m_size;
}

void ListBox::setSize(const glm::ivec2& size)
{
  m_size = size;
}

void ListBox::fitToContent()
{
  int y = 0;
  int maxHeight = 0;
  int maxWidth = 0;
  for(size_t i = 0; i < m_widgets.size(); ++i)
  {
    if(m_pageSize != 0 && i % m_pageSize == 0)
      y = 0;

    const auto& widget = m_widgets[i];
    widget->fitToContent();

    y += widget->getSize().y;
    maxHeight = std::max(maxHeight, y);
    maxWidth = std::max(maxWidth, widget->getSize().x);

    y += ui::OutlineBorderWidth;
  }

  m_size = {maxWidth, maxHeight};
}
} // namespace ui::widgets
