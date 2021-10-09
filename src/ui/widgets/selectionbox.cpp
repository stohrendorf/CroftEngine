#include "selectionbox.h"

#include "gridbox.h"
#include "label.h"
#include "ui/core.h"
#include "ui/ui.h"

#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <gl/pixel.h>
#include <iterator>

namespace ui::widgets
{
SelectionBox::SelectionBox(const std::string& message, const std::vector<std::string>& options, size_t initialSelection)
    : m_container{std::make_shared<GridBox>()}
    , m_selected{initialSelection}
{
  std::vector<std::string> lines;
  boost::split(lines, message, boost::is_any_of("\n"));
  std::transform(lines.begin(),
                 lines.end(),
                 std::back_inserter(m_messageLines),
                 [](const std::string& line)
                 {
                   return std::make_shared<Label>(line, Label::Alignment::Center);
                 });
  std::transform(options.begin(),
                 options.end(),
                 std::back_inserter(m_options),
                 [](const std::string& label)
                 {
                   return std::make_shared<Label>(label, Label::Alignment::Center);
                 });

  m_container->setExtents(1, m_messageLines.size() + m_options.size());
  for(size_t i = 0; i < m_messageLines.size(); ++i)
    m_container->set(0, i, m_messageLines[i]);
  for(size_t i = 0; i < m_options.size(); ++i)
    m_container->set(0, i + m_messageLines.size(), m_options[i]);
}

SelectionBox::~SelectionBox() = default;

void SelectionBox::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  const auto bgPos = m_position - glm::ivec2{ui::FontHeight, 2 * ui::FontHeight};
  const auto bgSize = m_size + glm::ivec2{2 * ui::FontHeight, 2 * ui::FontHeight};
  ui.drawBox(bgPos, bgSize, gl::SRGBA8{0, 0, 0, 224});
  ui.drawOutlineBox(bgPos, bgSize, 255);
  m_container->draw(ui, presenter);
}

void SelectionBox::update(bool hasFocus)
{
  for(size_t i = 0; i < m_options.size(); ++i)
    m_options[i]->update(hasFocus && m_selected == i);
}

void SelectionBox::setPosition(const glm::ivec2& position)
{
  m_position = position;
  m_container->setPosition(m_position);
}

glm::ivec2 SelectionBox::getPosition() const
{
  return m_position;
}

glm::ivec2 SelectionBox::getSize() const
{
  return m_size;
}

void SelectionBox::setSize(const glm::ivec2& size)
{
  m_size = size;
}

void SelectionBox::fitToContent()
{
  for(const auto& line : m_messageLines)
    line->fitToContent();
  for(const auto& option : m_options)
    option->fitToContent();
  m_container->fitToContent();
  for(const auto& line : m_messageLines)
    line->setSize({m_container->getColumnSizes()[0], line->getSize().y});
  for(const auto& option : m_options)
    option->setSize({m_container->getColumnSizes()[0], option->getSize().y});
  m_size = m_container->getSize();
}
} // namespace ui::widgets
