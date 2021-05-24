#include "controlsmenustate.h"

#include "core/i18n.h"
#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/names.h"
#include "menudisplay.h"
#include "ui/widgets/groupbox.h"
#include "ui/widgets/hbox.h"
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
  auto listBox = std::make_shared<ui::widgets::VBox>(glm::ivec2{0, 0}, glm::ivec2{0, 0});
  for(const auto& [action, key] : world.getEngine().getPresenter().getInputHandler().getKeyMap())
  {
    auto label
      = std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(key) + " :: " + hid::getName(action));
    listBox->append(label);
  }
  m_keyboardControls = std::make_shared<ui::widgets::GroupBox>(
    glm::ivec2{0, 0}, glm::ivec2{0, 0}, /* translators: TR charmap encoding */ _("Keyboard Controls"), listBox);
  m_keyboardControls->fitToContent();

  listBox = std::make_shared<ui::widgets::VBox>(glm::ivec2{0, 0}, glm::ivec2{0, 0});
  const auto& layout = world.getControllerLayouts().at("PS");
  for(const auto& [action, button] : world.getEngine().getPresenter().getInputHandler().getGamepadMap())
  {
    auto hbox = std::make_shared<ui::widgets::HBox>(glm::ivec2{0, 0}, glm::ivec2{0, 0});
    auto sprite = std::make_shared<ui::widgets::Sprite>(glm::ivec2{0, 0}, layout.at(button));
    hbox->append(sprite);
    auto label = std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(action));
    hbox->append(label);
    listBox->append(hbox);
  }
  m_gamepadControls = std::make_shared<ui::widgets::GroupBox>(
    glm::ivec2{0, 0}, glm::ivec2{0, 0}, /* translators: TR charmap encoding */ _("Gamepad Controls"), listBox);
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
