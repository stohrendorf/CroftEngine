#include "controlsmenustate.h"

#include "controlswidget.h"
#include "core/i18n.h"
#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/names.h"
#include "menudisplay.h"
#include "ui/widgets/gridbox.h"
#include "ui/widgets/label.h"

namespace menu
{
namespace
{
constexpr auto EditAction = hid::Action::Action;
constexpr auto BlinkPeriod = std::chrono::milliseconds{500};

template<typename T, typename U>
std::vector<T> getKeys(const std::map<T, U>& map, const U& needle)
{
  std::vector<T> keys;
  for(const auto& [key, value] : map)
  {
    if(value == needle)
      keys.emplace_back(key);
  }
  return keys;
}

std::shared_ptr<ControlsWidget> createControlsWidget(const engine::world::World& world)
{
  const auto createKeyLabel
    = [](const engine::InputMappingConfig& mappingConfig, hid::Action action) -> std::shared_ptr<ui::widgets::Widget>
  {
    const auto keys = getKeys(mappingConfig, engine::NamedAction{action});
    if(keys.empty())
    {
      return std::make_shared<ui::widgets::Label>(
        /* translators: TR charmap encoding */ pgettext("ButtonAssignment", "N/A"));
    }
    else
    {
      Expects(keys.size() == 1);
      if(std::holds_alternative<engine::NamedGlfwKey>(keys[0]))
      {
        return std::make_shared<ui::widgets::Label>(hid::getName(std::get<engine::NamedGlfwKey>(keys[0])));
      }
      else
      {
        return std::make_shared<ui::widgets::Label>(hid::getName(std::get<engine::NamedGlfwGamepadButton>(keys[0])));
      }
    }
  };
  return std::make_shared<ControlsWidget>(world.getEngine().getPresenter().getInputHandler().getMappings().at(0),
                                          createKeyLabel);
}
} // namespace

ControlsMenuState::ControlsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                     std::unique_ptr<MenuState> previous,
                                     const engine::world::World& world)
    : SelectedMenuState{ringTransform}
    , m_previous{std::move(previous)}
    , m_layout{std::make_shared<ui::widgets::GridBox>()}
    , m_editing{world.getPresenter().getInputHandler().getMappings()}
    , m_resetKey{hid::GlfwKey::Backspace, 5}
    , m_deleteKey{hid::GlfwKey::Delete, 2}
{
  m_layout->setExtents(1, 4);
  m_layout->set(0,
                1,
                std::make_shared<ui::widgets::Label>(
                  /* translators: TR charmap encoding */ _("To change a mapping, use %1%.", hid::getName(EditAction))));
  m_layout->set(0,
                2,
                std::make_shared<ui::widgets::Label>(
                  /* translators: TR charmap encoding */ _("To remove a mapping, hold %1% for %2% seconds.",
                                                           hid::getName(m_deleteKey.getKey()),
                                                           m_deleteKey.getDelay().count())));
  m_layout->set(0,
                3,
                std::make_shared<ui::widgets::Label>(
                  /* translators: TR charmap encoding */ _("To reset all mappings, hold %1% for %2% seconds.",
                                                           hid::getName(m_resetKey.getKey()),
                                                           m_resetKey.getDelay().count())));
  m_controls = createControlsWidget(world);
  m_layout->set(0, 0, m_controls);
}

std::unique_ptr<MenuState> ControlsMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& /*display*/)
{
  if(m_mode == Mode::Display && world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Menu))
  {
    return std::move(m_previous);
  }

  switch(m_mode)
  {
  case Mode::Display: handleDisplayInput(world); break;
  case Mode::ChangeKey: handleChangeKeyInput(world); break;
  }

  m_controls->fitToContent();
  m_layout->fitToContent();

  const auto vp = world.getPresenter().getViewport();
  m_layout->setPosition({(vp.x - m_layout->getSize().x) / 2, vp.y - 90 - m_layout->getSize().y});
  switch(m_mode)
  {
  case Mode::Display: m_layout->update(true); break;
  case Mode::ChangeKey:
    m_layout->update(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now())
                         .time_since_epoch()
                         .count()
                       % BlinkPeriod.count()
                     < BlinkPeriod.count() / 2);
    break;
  }
  m_layout->draw(ui, world.getPresenter());

  return nullptr;
}

void ControlsMenuState::handleDisplayInput(engine::world::World& world)
{
  if(m_resetKey.update(world.getPresenter().getInputHandler()))
  {
    world.getEngine().getEngineConfig()->resetInputMappings();
    m_editingIndex = 0;
    m_editing = world.getEngine().getEngineConfig()->inputMappings;
    world.getEngine().getPresenter().getInputHandler().setMappings(m_editing);
    m_controls->updateBindings(m_editing.at(m_editingIndex));
  }

  if(m_deleteKey.update(world.getPresenter().getInputHandler()))
  {
    auto& mapping = m_editing.at(m_editingIndex);
    auto keys = getKeys(mapping.mappings, engine::NamedAction{m_controls->getCurrentAction()});
    for(auto key : keys)
      mapping.mappings.erase(key);
    m_controls->updateBindings(mapping);
  }

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Backward))
  {
    m_controls->nextRow();
  }
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Forward))
  {
    m_controls->prevRow();
  }
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Right))
  {
    m_controls->nextColumn();
  }
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Left))
  {
    m_controls->prevColumn();
  }

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Action))
  {
    auto& mapping = m_editing.at(m_editingIndex);
    auto keys = getKeys(mapping.mappings, engine::NamedAction{m_controls->getCurrentAction()});
    m_mode = Mode::ChangeKey;
    (void)world.getPresenter().getInputHandler().takeRecentlyPressedKey();
  }
}

void ControlsMenuState::handleChangeKeyInput(engine::world::World& world)
{
  if(const auto newKey = world.getPresenter().getInputHandler().takeRecentlyPressedKey())
  {
    auto& mapping = m_editing.at(m_editingIndex);
    auto keys = getKeys(mapping.mappings, engine::NamedAction{m_controls->getCurrentAction()});
    for(auto key : keys)
      mapping.mappings.erase(key);
    mapping.mappings[newKey.value()] = m_controls->getCurrentAction();
    m_mode = Mode::Display;
    m_controls->updateBindings(mapping);
  }
}
} // namespace menu
