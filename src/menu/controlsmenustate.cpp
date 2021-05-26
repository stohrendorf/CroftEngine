#include "controlsmenustate.h"

#include "core/i18n.h"
#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/names.h"
#include "menudisplay.h"
#include "ui/widgets/gridbox.h"
#include "ui/widgets/groupbox.h"
#include "ui/widgets/label.h"
#include "ui/widgets/sprite.h"
#include "ui/widgets/vbox.h"

namespace menu
{
ControlsMenuState::ControlsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                     std::unique_ptr<MenuState> previous,
                                     const engine::world::World& world)
    : SelectedMenuState{ringTransform}
    , m_previous{std::move(previous)}
{
  auto gridBox = std::make_shared<ui::widgets::GridBox>(
    glm::ivec2{0, 0}, glm::ivec2{0, 0}, glm::ivec2{10, ui::OutlineBorderWidth});
  gridBox->setExtents(8, 4);
  const auto createKeyLabel = [&world](hid::Action action)
  {
    const auto& keyMap = world.getEngine().getPresenter().getInputHandler().getKeyMap();
    auto it = keyMap.find(action);
    if(it == keyMap.end())
      return std::make_shared<ui::widgets::Label>(
        glm::ivec2{0, 0}, /* translators: TR charcmap encoding */ pgettext("ButtonAssignment", "N/A"));
    return std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(it->second));
  };
  gridBox->set(0, 0, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::Forward)));
  gridBox->set(1, 0, createKeyLabel(hid::Action::Forward));
  gridBox->set(0, 1, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::Backward)));
  gridBox->set(1, 1, createKeyLabel(hid::Action::Backward));
  gridBox->set(0, 2, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::Left)));
  gridBox->set(1, 2, createKeyLabel(hid::Action::Left));
  gridBox->set(0, 3, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::Right)));
  gridBox->set(1, 3, createKeyLabel(hid::Action::Right));

  gridBox->set(3, 0, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::StepLeft)));
  gridBox->set(4, 0, createKeyLabel(hid::Action::StepLeft));
  gridBox->set(3, 1, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::StepRight)));
  gridBox->set(4, 1, createKeyLabel(hid::Action::StepRight));
  gridBox->set(3, 2, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::MoveSlow)));
  gridBox->set(4, 2, createKeyLabel(hid::Action::MoveSlow));
  gridBox->set(3, 3, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::Roll)));
  gridBox->set(4, 3, createKeyLabel(hid::Action::Roll));

  gridBox->set(6, 0, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::Action)));
  gridBox->set(7, 0, createKeyLabel(hid::Action::Action));
  gridBox->set(6, 1, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::Holster)));
  gridBox->set(7, 1, createKeyLabel(hid::Action::Holster));
  gridBox->set(6, 2, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::Menu)));
  gridBox->set(7, 2, createKeyLabel(hid::Action::Menu));
  gridBox->set(6, 3, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::FreeLook)));
  gridBox->set(7, 3, createKeyLabel(hid::Action::FreeLook));

  m_keyboardControls = std::make_shared<ui::widgets::GroupBox>(
    glm::ivec2{0, 0}, glm::ivec2{0, 0}, /* translators: TR charmap encoding */ _("Keyboard Controls"), gridBox);
  m_keyboardControls->fitToContent();

  gridBox = std::make_shared<ui::widgets::GridBox>(
    glm::ivec2{0, 0}, glm::ivec2{0, 0}, glm::ivec2{10, ui::OutlineBorderWidth});
  gridBox->setExtents(8, 4);

  const auto createButtonLabel
    = [&world](const engine::ControllerLayout& layout, hid::Action action) -> std::shared_ptr<ui::widgets::Widget>
  {
    const auto& buttonMap = world.getEngine().getPresenter().getInputHandler().getGamepadMap();
    auto it = buttonMap.find(action);
    if(it == buttonMap.end())
      return std::make_shared<ui::widgets::Label>(
        glm::ivec2{0, 0}, /* translators: TR charcmap encoding */ pgettext("ButtonAssignment", "N/A"));
    return std::make_shared<ui::widgets::Sprite>(glm::ivec2{0, 0}, layout.at(it->second));
  };
  const auto& layout = world.getControllerLayouts().at("PS");
  gridBox->set(0, 0, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::Forward)));
  gridBox->set(1, 0, createButtonLabel(layout, hid::Action::Forward));
  gridBox->set(0, 1, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::Backward)));
  gridBox->set(1, 1, createButtonLabel(layout, hid::Action::Backward));
  gridBox->set(0, 2, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::Left)));
  gridBox->set(1, 2, createButtonLabel(layout, hid::Action::Left));
  gridBox->set(0, 3, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::Right)));
  gridBox->set(1, 3, createButtonLabel(layout, hid::Action::Right));

  gridBox->set(3, 0, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::StepLeft)));
  gridBox->set(4, 0, createButtonLabel(layout, hid::Action::StepLeft));
  gridBox->set(3, 1, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::StepRight)));
  gridBox->set(4, 1, createButtonLabel(layout, hid::Action::StepRight));
  gridBox->set(3, 2, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::MoveSlow)));
  gridBox->set(4, 2, createButtonLabel(layout, hid::Action::MoveSlow));
  gridBox->set(3, 3, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::Roll)));
  gridBox->set(4, 3, createButtonLabel(layout, hid::Action::Roll));

  gridBox->set(6, 0, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::Action)));
  gridBox->set(7, 0, createButtonLabel(layout, hid::Action::Action));
  gridBox->set(6, 1, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::Holster)));
  gridBox->set(7, 1, createButtonLabel(layout, hid::Action::Holster));
  gridBox->set(6, 2, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::Menu)));
  gridBox->set(7, 2, createButtonLabel(layout, hid::Action::Menu));
  gridBox->set(6, 3, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(hid::Action::FreeLook)));
  gridBox->set(7, 3, createButtonLabel(layout, hid::Action::FreeLook));

  m_gamepadControls = std::make_shared<ui::widgets::GroupBox>(
    glm::ivec2{0, 0}, glm::ivec2{0, 0}, /* translators: TR charmap encoding */ _("Gamepad Controls"), gridBox);
  m_gamepadControls->fitToContent();
}

std::unique_ptr<MenuState> ControlsMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& /*display*/)
{
  const auto vp = world.getPresenter().getViewport();
  {
    m_keyboardControls->setPosition(
      {vp.x / 2 - m_keyboardControls->getSize().x - 10, vp.y - 90 - m_keyboardControls->getSize().y});
    m_keyboardControls->draw(ui, world.getPresenter());
  }
  {
    m_gamepadControls->setPosition({vp.x / 2 + 10, vp.y - 90 - m_gamepadControls->getSize().y});
    m_gamepadControls->draw(ui, world.getPresenter());
  }

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Menu))
  {
    return std::move(m_previous);
  }

  return nullptr;
}
} // namespace menu
