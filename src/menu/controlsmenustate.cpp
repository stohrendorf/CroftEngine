#include "controlsmenustate.h"

#include "core/i18n.h"
#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/names.h"
#include "menudisplay.h"
#include "ui/widgets/gridbox.h"
#include "ui/widgets/label.h"
#include "ui/widgets/sprite.h"

#include <functional>

namespace menu
{
namespace
{
void applyLayout(const std::shared_ptr<ui::widgets::GridBox>& gridBox)
{
  gridBox->fitToContent();
  gridBox->setRowSize(3, 2 * ui::FontHeight);
  gridBox->setColumnSize(1, gridBox->getColumnSizes()[1] + 2 * ui::FontHeight);
  gridBox->setColumnSize(3, gridBox->getColumnSizes()[3] + 2 * ui::FontHeight);
}

std::shared_ptr<ui::widgets::GridBox>
  createButtonGridBox(const std::function<std::shared_ptr<ui::widgets::Widget>(hid::Action)>& factory)
{
  auto gridBox = std::make_shared<ui::widgets::GridBox>(
    glm::ivec2{0, 0}, glm::ivec2{0, 0}, glm::ivec2{10, ui::OutlineBorderWidth});

  auto add = [&gridBox, &factory](size_t x0, size_t y, hid::Action action)
  {
    auto label = std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(action));
    label->fitToContent();
    gridBox->set(x0, y, label);

    auto widget = factory(action);
    widget->fitToContent();
    gridBox->set(x0 + 1, y, widget);
  };

  gridBox->setExtents(6, 8);

  add(0, 0, hid::Action::Forward);
  add(0, 1, hid::Action::Backward);
  add(0, 2, hid::Action::Left);
  add(0, 3, hid::Action::Right);

  add(2, 0, hid::Action::StepLeft);
  add(2, 1, hid::Action::StepRight);
  add(2, 2, hid::Action::MoveSlow);
  add(2, 3, hid::Action::Roll);

  add(4, 0, hid::Action::Action);
  add(4, 1, hid::Action::Holster);
  add(4, 2, hid::Action::Menu);
  add(4, 3, hid::Action::FreeLook);

  add(0, 4, hid::Action::DrawPistols);
  add(0, 5, hid::Action::DrawShotgun);
  add(0, 6, hid::Action::DrawUzis);
  add(0, 7, hid::Action::DrawMagnums);

  add(2, 4, hid::Action::ConsumeSmallMedipack);
  add(2, 5, hid::Action::ConsumeLargeMedipack);

  add(4, 4, hid::Action::Save);
  add(4, 5, hid::Action::Load);
  add(4, 6, hid::Action::Screenshot);

  applyLayout(gridBox);
  return gridBox;
}
} // namespace

ControlsMenuState::ControlsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                     std::unique_ptr<MenuState> previous,
                                     const engine::world::World& world)
    : SelectedMenuState{ringTransform}
    , m_previous{std::move(previous)}
    , m_allControls{std::make_shared<ui::widgets::GridBox>(glm::ivec2{0, 0}, glm::ivec2{0, 0})}
{
  m_allControls->setExtents(1, 2);

  const auto createKeyLabel = [&world](hid::Action action) -> std::shared_ptr<ui::widgets::Widget>
  {
    const auto& keyMap = world.getEngine().getPresenter().getInputHandler().getKeyMap();
    auto it = keyMap.find(action);
    if(it == keyMap.end())
      return std::make_shared<ui::widgets::Label>(
        glm::ivec2{0, 0}, /* translators: TR charcmap encoding */ pgettext("ButtonAssignment", "N/A"));
    return std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(it->second));
  };
  m_controls.emplace_back(createButtonGridBox(createKeyLabel));

  const auto& layout = world.getControllerLayouts().at("PS");
  const auto createButtonLabel = [&world, &layout](hid::Action action) -> std::shared_ptr<ui::widgets::Widget>
  {
    const auto& buttonMap = world.getEngine().getPresenter().getInputHandler().getGamepadMap();
    auto it = buttonMap.find(action);
    if(it == buttonMap.end())
      return std::make_shared<ui::widgets::Label>(
        glm::ivec2{0, 0}, /* translators: TR charcmap encoding */ pgettext("ButtonAssignment", "N/A"));
    return std::make_shared<ui::widgets::Sprite>(glm::ivec2{0, 0}, glm::ivec2{0, 0}, layout.at(it->second));
  };
  m_controls.emplace_back(createButtonGridBox(createButtonLabel));

  // align columns
  for(size_t x = 0; x < std::get<0>(m_controls[0]->getExtents()); ++x)
  {
    int maxColumnWidth = 0;
    for(const auto& controlBox : m_controls)
    {
      maxColumnWidth = std::max(maxColumnWidth, controlBox->getColumnSizes()[x]);
    }
    for(const auto& controlBox : m_controls)
    {
      controlBox->setColumnSize(x, maxColumnWidth);
    }
  }

  auto groupBox = std::make_shared<ui::widgets::GroupBox>(
    glm::ivec2{0, 0}, glm::ivec2{0, 0}, /* translators: TR charmap encoding */ _("Keyboard"), m_controls[0]);
  groupBox->fitToContent();
  m_allControls->set(0, 0, groupBox);

  groupBox = std::make_shared<ui::widgets::GroupBox>(
    glm::ivec2{0, 0}, glm::ivec2{0, 0}, /* translators: TR charmap encoding */ _("Gamepad"), m_controls[1]);
  groupBox->fitToContent();
  m_allControls->set(0, 1, groupBox);

  m_allControls->fitToContent();
  m_controls[0]->setSelected({1, 0});
  m_controls[1]->setSelected({1, 0});
}

std::unique_ptr<MenuState> ControlsMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& /*display*/)
{
  {
    const auto& currentControls = m_controls.at(std::get<1>(m_allControls->getSelected()));

    if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Backward))
    {
      if(!currentControls->nextRow())
      {
        if(!m_allControls->nextRow())
        {
          // wrap around
          m_allControls->setSelected({std::get<0>(m_allControls->getSelected()), 0});
        }

        const auto& nextControls = m_controls.at(std::get<1>(m_allControls->getSelected()));
        nextControls->setSelected({std::get<0>(currentControls->getSelected()), 0});
      }
    }
    else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Forward))
    {
      if(!currentControls->prevRow())
      {
        if(!m_allControls->prevRow())
        {
          // wrap around
          m_allControls->setSelected(
            {std::get<0>(m_allControls->getSelected()), std::get<1>(m_allControls->getExtents()) - 1});
        }

        const auto& nextControls = m_controls.at(std::get<1>(m_allControls->getSelected()));
        nextControls->setSelected(
          {std::get<0>(currentControls->getSelected()), std::get<1>(nextControls->getExtents()) - 1});
      }
    }
    else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Right))
    {
      if(!currentControls->nextColumn() || !currentControls->nextColumn())
      {
        currentControls->setSelected({1, std::get<1>(currentControls->getSelected())});
      }
    }
    else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Left))
    {
      if(!currentControls->prevColumn() || !currentControls->prevColumn())
      {
        currentControls->setSelected(
          {std::get<0>(currentControls->getExtents()) - 1, std::get<1>(currentControls->getSelected())});
      }
    }
  }

  const auto vp = world.getPresenter().getViewport();
  m_allControls->setPosition({(vp.x - m_allControls->getSize().x) / 2, vp.y - 90 - m_allControls->getSize().y});
  m_allControls->update(true);
  m_allControls->draw(ui, world.getPresenter());

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Menu))
  {
    return std::move(m_previous);
  }

  return nullptr;
}
} // namespace menu
