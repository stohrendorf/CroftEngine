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
std::shared_ptr<ui::widgets::GridBox>
  createButtonGridBox(const std::function<std::shared_ptr<ui::widgets::Widget>(hid::Action)>& factory)
{
  auto gridBox = std::make_shared<ui::widgets::GridBox>(
    glm::ivec2{0, 0}, glm::ivec2{0, 0}, glm::ivec2{10, ui::OutlineBorderWidth});

  auto add = [&gridBox, &factory](size_t x0, size_t y, hid::Action action)
  {
    gridBox->set(x0, y, std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(action)));
    gridBox->set(x0 + 1, y, factory(action));
  };

  gridBox->setExtents(8, 9);

  add(0, 0, hid::Action::Forward);
  add(0, 1, hid::Action::Backward);
  add(0, 2, hid::Action::Left);
  add(0, 3, hid::Action::Right);

  add(3, 0, hid::Action::StepLeft);
  add(3, 1, hid::Action::StepRight);
  add(3, 2, hid::Action::MoveSlow);
  add(3, 3, hid::Action::Roll);

  add(6, 0, hid::Action::Action);
  add(6, 1, hid::Action::Holster);
  add(6, 2, hid::Action::Menu);
  add(6, 3, hid::Action::FreeLook);

  add(0, 5, hid::Action::DrawPistols);
  add(0, 6, hid::Action::DrawShotgun);
  add(0, 7, hid::Action::DrawUzis);
  add(0, 8, hid::Action::DrawMagnums);

  add(3, 5, hid::Action::ConsumeSmallMedipack);
  add(3, 6, hid::Action::ConsumeLargeMedipack);

  add(6, 5, hid::Action::Save);
  add(6, 6, hid::Action::Load);
  add(6, 7, hid::Action::Screenshot);

  return gridBox;
}
} // namespace

ControlsMenuState::ControlsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                     std::unique_ptr<MenuState> previous,
                                     const engine::world::World& world)
    : SelectedMenuState{ringTransform}
    , m_previous{std::move(previous)}
{
  const auto createKeyLabel = [&world](hid::Action action) -> std::shared_ptr<ui::widgets::Widget>
  {
    const auto& keyMap = world.getEngine().getPresenter().getInputHandler().getKeyMap();
    auto it = keyMap.find(action);
    if(it == keyMap.end())
      return std::make_shared<ui::widgets::Label>(
        glm::ivec2{0, 0}, /* translators: TR charcmap encoding */ pgettext("ButtonAssignment", "N/A"));
    return std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(it->second));
  };
  auto gridBox = createButtonGridBox(createKeyLabel);
  m_keyboardControls = std::make_shared<ui::widgets::GroupBox>(
    glm::ivec2{0, 0}, glm::ivec2{0, 0}, /* translators: TR charmap encoding */ _("Keyboard Controls"), gridBox);
  m_keyboardControls->fitToContent();

  const auto& layout = world.getControllerLayouts().at("PS");
  const auto createButtonLabel = [&world, &layout](hid::Action action) -> std::shared_ptr<ui::widgets::Widget>
  {
    const auto& buttonMap = world.getEngine().getPresenter().getInputHandler().getGamepadMap();
    auto it = buttonMap.find(action);
    if(it == buttonMap.end())
      return std::make_shared<ui::widgets::Label>(
        glm::ivec2{0, 0}, /* translators: TR charcmap encoding */ pgettext("ButtonAssignment", "N/A"));
    return std::make_shared<ui::widgets::Sprite>(glm::ivec2{0, 0}, layout.at(it->second));
  };
  gridBox = createButtonGridBox(createButtonLabel);

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
