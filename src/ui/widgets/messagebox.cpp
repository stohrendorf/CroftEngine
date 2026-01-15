#include "messagebox.h"

#include "core/i18n.h"
#include "label.h"
#include "ui/core.h"
#include "ui/ui.h"

#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <gl/pixel.h>
#include <gsl-lite/gsl-lite.hpp>
#include <gslu.h>
#include <iterator>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

namespace ui::widgets
{
namespace
{
[[nodiscard]] std::vector<gslu::nn_shared<Label>> stringToLabels(const std::string& text)
{
  std::vector<std::string> lines;
  boost::algorithm::split(lines,
                          text,
                          [](const char c)
                          {
                            return c == '\n';
                          });
  std::vector<gslu::nn_shared<Label>> labels;
  std::ranges::transform(lines,
                         std::back_inserter(labels),
                         [](const std::string& line)
                         {
                           return gsl_lite::make_shared<Label>(line);
                         });
  return labels;
}

[[nodiscard]] std::vector<gslu::nn_shared<Label>> linesToLabels(const std::vector<std::string>& lines)
{
  std::vector<gslu::nn_shared<Label>> labels;
  std::ranges::transform(lines,
                         std::back_inserter(labels),
                         [](const std::string& line)
                         {
                           return gsl_lite::make_shared<Label>(line);
                         });
  return labels;
}
} // namespace

MessageBox::MessageBox(const std::string& label)
    : m_questions{stringToLabels(label)}
    , m_yes{std::make_shared<Label>(/* translators: TR charmap encoding */ _("Yes"))}
    , m_no{std::make_shared<Label>(/* translators: TR charmap encoding */ _("No"))}
{
}

MessageBox::MessageBox(const std::vector<std::string>& labels)
    : m_questions{linesToLabels(labels)}
    , m_yes{std::make_shared<Label>(/* translators: TR charmap encoding */ _("Yes"))}
    , m_no{std::make_shared<Label>(/* translators: TR charmap encoding */ _("No"))}
{
}

MessageBox::~MessageBox() = default;

void MessageBox::draw(Ui& ui, const engine::Presenter& presenter) const
{
  const auto center = m_position.x + getSize().x / 2;
  int y = m_position.y + FontHeight * 3 / 2;
  for(const auto& q : m_questions)
  {
    q->setPosition({m_position.x + (getSize().x - q->getSize().x) / 2, y});
    y += FontHeight;
  }
  const auto answersWidth = m_yes->getSize().x + m_no->getSize().x + FontHeight;
  const auto answersX = center - answersWidth / 2 - FontHeight / 2;
  const auto answersY = m_position.y + FontHeight * 5 / 2 + FontHeight * m_questions.size();
  m_yes->setPosition({answersX, answersY});
  m_no->setPosition({answersX + m_yes->getSize().x + FontHeight, answersY});

  ui.drawBox(m_position, getSize(), gl::SRGBA8{0, 0, 0, DefaultBackgroundAlpha});
  ui.drawOutlineBox(m_position, getSize());
  for(const auto& q : m_questions)
  {
    q->draw(ui, presenter);
  }
  m_yes->draw(ui, presenter);
  m_no->draw(ui, presenter);
}

void MessageBox::tick(const bool hasFocus)
{
  m_yes->tick(hasFocus && m_confirmed);
  m_no->tick(hasFocus && !m_confirmed);
}

void MessageBox::setPosition(const glm::ivec2& position)
{
  m_position = position;
}

glm::ivec2 MessageBox::getPosition() const
{
  return m_position;
}

glm::ivec2 MessageBox::getSize() const
{
  const auto width = std::accumulate(m_questions.begin(),
                                     m_questions.end(),
                                     0,
                                     [](int width, auto& q)
                                     {
                                       return std::max(width, q->getSize().x);
                                     });
  return {width + 2 * FontHeight, (3 + m_questions.size()) * FontHeight};
}

void MessageBox::setSize(const glm::ivec2& /*size*/)
{
}

void MessageBox::fitToContent()
{
}
} // namespace ui::widgets