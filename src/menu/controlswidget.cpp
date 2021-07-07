#include "controlswidget.h"

#include "core/i18n.h"
#include "hid/actions.h"
#include "hid/names.h"
#include "ui/widgets/gridbox.h"
#include "ui/widgets/groupbox.h"
#include "ui/widgets/label.h"

namespace menu
{
ControlsWidget::ControlsWidget(const std::string& title,
                               const std::function<std::shared_ptr<Widget>(hid::Action)>& factory)
    : m_content{std::make_shared<ui::widgets::GridBox>()}
    , m_container{std::make_shared<ui::widgets::GroupBox>(title, m_content)}
{
  m_content->setExtents(1, 2);

  auto gridBox = std::make_shared<ui::widgets::GridBox>(glm::ivec2{10, ui::OutlineBorderWidth});
  gridBox->setExtents(Columns, 5);
  gridBox->setSelected({1, 0});
  m_controlGroups.emplace_back(gridBox);

  auto groupBox = std::make_shared<ui::widgets::GroupBox>(
    /* translators: TR charmap encoding */ _("Gameplay"), gridBox);
  m_content->set(0, 0, groupBox);

  auto add = [&gridBox, &factory](size_t x0, size_t y, hid::Action action)
  {
    auto label = std::make_shared<ui::widgets::Label>(hid::getName(action));
    label->fitToContent();
    gridBox->set(x0, y, label);

    auto widget = factory(action);
    widget->fitToContent();
    gridBox->set(x0 + 1, y, widget);
  };

  add(0, 0, hid::Action::Forward);
  add(0, 1, hid::Action::Backward);
  add(0, 2, hid::Action::Left);
  add(0, 3, hid::Action::Right);
  add(0, 4, hid::Action::Jump);

  add(2, 0, hid::Action::StepLeft);
  add(2, 1, hid::Action::StepRight);
  add(2, 2, hid::Action::Walk);
  add(2, 3, hid::Action::Roll);
  add(2, 4, hid::Action::FreeLook);

  add(4, 0, hid::Action::Action);
  add(4, 1, hid::Action::Holster);
  add(4, 2, hid::Action::Menu);

  gridBox = std::make_shared<ui::widgets::GridBox>(glm::ivec2{10, ui::OutlineBorderWidth});
  gridBox->setExtents(Columns, 4);
  m_controlGroups.emplace_back(gridBox);

  groupBox = std::make_shared<ui::widgets::GroupBox>(/* translators: TR charmap encoding */ _("Shortcuts"), gridBox);
  m_content->set(0, 1, groupBox);

  add(0, 0, hid::Action::DrawPistols);
  add(0, 1, hid::Action::DrawShotgun);
  add(0, 2, hid::Action::DrawUzis);
  add(0, 3, hid::Action::DrawMagnums);

  add(2, 0, hid::Action::ConsumeSmallMedipack);
  add(2, 1, hid::Action::ConsumeLargeMedipack);

  add(4, 0, hid::Action::Save);
  add(4, 1, hid::Action::Load);
  add(4, 2, hid::Action::Screenshot);

  fitToContent();
}

void ControlsWidget::fitToContent()
{
  for(const auto& gridBox : m_controlGroups)
  {
    gridBox->fitToContent();
    gridBox->setColumnSize(1, gridBox->getColumnSizes()[1] + 2 * ui::FontHeight);
    gridBox->setColumnSize(3, gridBox->getColumnSizes()[3] + 2 * ui::FontHeight);
  }

  for(size_t x = 0; x < Columns; ++x)
  {
    int maxWidth = 0;
    for(const auto& gridBox : m_controlGroups)
    {
      maxWidth = std::max(gridBox->getColumnSizes().at(x), maxWidth);
    }
    for(const auto& gridBox : m_controlGroups)
    {
      gridBox->setColumnSize(x, maxWidth);
    }
  }

  for(size_t x = 0; x < std::get<0>(m_content->getExtents()); ++x)
    for(size_t y = 0; y < std::get<1>(m_content->getExtents()); ++y)
      m_content->getWidget(x, y)->fitToContent();

  m_content->fitToContent();
  m_container->fitToContent();
}

glm::ivec2 ControlsWidget::getPosition() const
{
  return m_container->getPosition();
}

glm::ivec2 ControlsWidget::getSize() const
{
  return m_container->getSize();
}

void ControlsWidget::setPosition(const glm::ivec2& position)
{
  m_container->setPosition(position);
}

void ControlsWidget::setSize(const glm::ivec2& size)
{
  m_container->setSize(size);
}

void ControlsWidget::update(bool hasFocus)
{
  m_container->update(hasFocus);
}

void ControlsWidget::draw(ui::Ui& ui, const engine::Presenter& presenter) const
{
  m_container->draw(ui, presenter);
}

const std::shared_ptr<ui::widgets::GridBox>& ControlsWidget::getCurrentGridBox() const
{
  return m_controlGroups.at(std::get<1>(m_content->getSelected()));
}

void ControlsWidget::nextRow()
{
  do
  {
    const auto& currentGridBox = getCurrentGridBox();
    if(!currentGridBox->nextRow())
    {
      if(!m_content->nextRow())
      {
        // wrap around
        m_content->setSelected({0, 0});
      }

      getCurrentGridBox()->setSelected({std::get<0>(currentGridBox->getSelected()), 0});
    }
  } while(getCurrentGridBox()->getSelectedWidget() == nullptr);
}

void ControlsWidget::prevRow()
{
  do
  {
    const auto& currentGridBox = getCurrentGridBox();
    if(!currentGridBox->prevRow())
    {
      if(!m_content->prevRow())
      {
        // wrap around
        m_content->setSelected({0, m_controlGroups.size() - 1});
      }

      getCurrentGridBox()->setSelected(
        {std::get<0>(currentGridBox->getSelected()), std::get<1>(getCurrentGridBox()->getExtents()) - 1});
    }
  } while(getCurrentGridBox()->getSelectedWidget() == nullptr);
}

void ControlsWidget::nextColumn()
{
  do
  {
    const auto& currentGridBox = getCurrentGridBox();
    if(!currentGridBox->nextColumn() || !currentGridBox->nextColumn())
    {
      getCurrentGridBox()->setSelected({1, std::get<1>(currentGridBox->getSelected())});
    }
  } while(getCurrentGridBox()->getSelectedWidget() == nullptr);
}

void ControlsWidget::prevColumn()
{
  do
  {
    const auto& currentGridBox = getCurrentGridBox();
    if(!currentGridBox->prevColumn() || !currentGridBox->prevColumn())
    {
      getCurrentGridBox()->setSelected(
        {std::get<0>(currentGridBox->getExtents()) - 1, std::get<1>(currentGridBox->getSelected())});
    }
  } while(getCurrentGridBox()->getSelectedWidget() == nullptr);
}
} // namespace menu
