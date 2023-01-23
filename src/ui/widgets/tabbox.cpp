#include "tabbox.h"

#include "ui/boxgouraud.h"
#include "ui/core.h"
#include "ui/text.h"
#include "ui/ui.h"

#include <engine/presenter.h>

namespace ui::widgets
{

constexpr int TitleOffset = 10;
constexpr int InnerMargin = 10;
constexpr int WidgetOffsetTop = ui::FontHeight / 2 + InnerMargin + ui::OutlineBorderWidth;
constexpr int TotalVerticalMargin = WidgetOffsetTop + InnerMargin + 2 * ui::OutlineBorderWidth;

constexpr int checkboxSize = 6;

Tab::Tab(const std::string& title)
    : m_title(std::make_unique<Text>(title))
{
}

Tab::~Tab() = default;

void Tab::update(bool hasFocus)
{
  m_hasFocus = hasFocus;
}

void Tab::fitToContent()
{
  m_size = m_title->getWidth() + glm::ivec2{2 * InnerMargin + checkboxSize, WidgetOffsetTop};
}

void Tab::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  const auto bgPos = m_position - glm::ivec2{OutlineBorderWidth - TitleOffset, FontHeight - 1};
  const auto bgSize = glm::ivec2{m_title->getWidth() + 2 * OutlineBorderWidth + checkboxSize * 2, ui::FontHeight};
  ui.drawBox(bgPos, bgSize, gl::SRGBA8{0, 0, 0, 255});
  ui.drawOutlineBox(bgPos, bgSize);
  m_title->draw(ui, presenter.getTrFont(), (m_position + glm::ivec2{InnerMargin + checkboxSize * 2, 0}));
  if(m_hasFocus)
  {
    ui.drawBox(bgPos + glm::ivec2{5, 6}, glm::ivec2{checkboxSize, checkboxSize}, 15);
  }
}

TabBox::TabBox() = default;
TabBox::~TabBox() = default;

void TabBox::fitToContent()
{
  if(m_tabs.empty())
  {
    m_size = {0, 0};
    return;
  }

  int maxHeight = 0;
  int maxWidth = 0;

  for(const auto& [container, content] : m_tabs)
  {
    content->fitToContent();
    maxHeight = std::max(maxHeight, content->getSize().y);
    maxWidth = std::max(maxWidth, content->getSize().x);
  }

  m_size = {maxWidth, maxHeight + ui::FontHeight};
}

void TabBox::update(bool /*hasFocus*/)
{
  for(size_t i = 0; i < m_tabs.size(); i++)
  {
    m_tabs[i].container->update(i == m_selectedTabIndex);
  }
}

void TabBox::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  int tabOffsetX = 0;
  for(const auto& [container, content] : m_tabs)
  {
    container->fitToContent();
    container->setPosition(m_position + glm::ivec2{tabOffsetX, 0});
    tabOffsetX += container->getSize().x;
  }

  {
    const auto bgPos = m_position - glm::ivec2{OutlineBorderWidth, FontHeight / 2 - 1};
    const auto bgSize = m_size + glm::ivec2{TitleOffset, TotalVerticalMargin};

    const auto g = makeBackgroundCircle(gl::SRGBA8{0, 128, 0, 192}, gl::SRGBA8{0, 0, 0, 192});
    g.draw(ui, bgPos, bgSize);

    ui.drawOutlineBox(bgPos, bgSize);
  }
  for(const auto& [container, content] : m_tabs)
  {
    container->draw(ui, presenter);
  }
  const auto& content = m_tabs[m_selectedTabIndex].content;
  content->setPosition(m_position + glm::ivec2{0, TotalVerticalMargin});
  content->draw(ui, presenter);
}

void TabBox::addTab(const gslu::nn_shared<Tab>& tab, const gslu::nn_shared<Widget>& content)
{
  m_tabs.emplace_back(ContainerContent{tab, content});
}
} //namespace ui::widgets
