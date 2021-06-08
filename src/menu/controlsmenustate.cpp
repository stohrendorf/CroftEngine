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
    auto label = std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(action));
    label->fitToContent();
    gridBox->set(x0, y, label);

    auto widget = factory(action);
    widget->fitToContent();
    gridBox->set(x0 + 1, y, widget);
  };

  gridBox->setExtents(8, 8);

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

  add(0, 4, hid::Action::DrawPistols);
  add(0, 5, hid::Action::DrawShotgun);
  add(0, 6, hid::Action::DrawUzis);
  add(0, 7, hid::Action::DrawMagnums);

  add(3, 4, hid::Action::ConsumeSmallMedipack);
  add(3, 5, hid::Action::ConsumeLargeMedipack);

  add(6, 4, hid::Action::Save);
  add(6, 5, hid::Action::Load);
  add(6, 6, hid::Action::Screenshot);

  gridBox->fitToContent();
  gridBox->setRowSize(4, 2 * ui::FontHeight);
  return gridBox;
}
} // namespace

ControlsMenuState::ControlsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                     std::unique_ptr<MenuState> previous,
                                     const engine::world::World& world)
    : SelectedMenuState{ringTransform}
    , m_previous{std::move(previous)}
    , m_controls{std::make_shared<ui::widgets::GridBox>(glm::ivec2{0, 0}, glm::ivec2{0, 0})}
{
  m_controls->setExtents(1, 2);

  const auto createKeyLabel = [&world](hid::Action action) -> std::shared_ptr<ui::widgets::Widget>
  {
    const auto& keyMap = world.getEngine().getPresenter().getInputHandler().getKeyMap();
    auto it = keyMap.find(action);
    if(it == keyMap.end())
      return std::make_shared<ui::widgets::Label>(
        glm::ivec2{0, 0}, /* translators: TR charcmap encoding */ pgettext("ButtonAssignment", "N/A"));
    return std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(it->second));
  };
  auto gridBox1 = createButtonGridBox(createKeyLabel);

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
  auto gridBox2 = createButtonGridBox(createButtonLabel);

  // align columns
  Expects(gridBox1->getExtents().first == gridBox2->getExtents().first);
  for(size_t x = 0; x < gridBox1->getExtents().first; ++x)
  {
    auto w = std::max(gridBox1->getColumnSizes()[x], gridBox2->getColumnSizes()[x]);
    gridBox1->setColumnSize(x, w);
    gridBox2->setColumnSize(x, w);
  }

  auto groupBox = std::make_shared<ui::widgets::GroupBox>(
    glm::ivec2{0, 0}, glm::ivec2{0, 0}, /* translators: TR charmap encoding */ _("Keyboard"), gridBox1);
  groupBox->fitToContent();
  m_controls->set(0, 0, groupBox);

  groupBox = std::make_shared<ui::widgets::GroupBox>(
    glm::ivec2{0, 0}, glm::ivec2{0, 0}, /* translators: TR charmap encoding */ _("Gamepad"), gridBox2);
  groupBox->fitToContent();
  m_controls->set(0, 1, groupBox);

  m_controls->fitToContent();
}

std::unique_ptr<MenuState> ControlsMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& /*display*/)
{
  const auto vp = world.getPresenter().getViewport();
  m_controls->setPosition({(vp.x - m_controls->getSize().x) / 2, vp.y - 90 - m_controls->getSize().y});
  m_controls->draw(ui, world.getPresenter());

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Menu))
  {
    return std::move(m_previous);
  }

  return nullptr;
}
} // namespace menu
