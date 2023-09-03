#include "controlsmenustate.h"

#include "controlswidget.h"
#include "core/i18n.h"
#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/glfw_axes.h"
#include "hid/glfw_axis_dirs.h"
#include "hid/glfw_gamepad_buttons.h"
#include "hid/glfw_keys.h"
#include "hid/inputhandler.h"
#include "hid/inputstate.h"
#include "hid/names.h"
#include "menu/selectedmenustate.h"
#include "menustate.h"
#include "serialization/named_enum.h"
#include "ui/ui.h"
#include "ui/widgets/gridbox.h"
#include "ui/widgets/label.h"
#include "ui/widgets/selectionbox.h"
#include "ui/widgets/sprite.h"

#include <algorithm>
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <chrono>
#include <functional>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <initializer_list>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace ui::widgets
{
class Widget;
}

namespace menu
{
namespace
{
constexpr auto EditAction = hid::Action::PrimaryInteraction;
constexpr auto ChangeControllerTypeAction = hid::Action::SecondaryInteraction;
constexpr auto PrevProfileAction = hid::Action::PrevScreen;
constexpr auto NextProfileAction = hid::Action::NextScreen;
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

std::function<std::shared_ptr<ui::widgets::Widget>(
  const engine::InputMappingConfig&, const engine::InputMappingConfig&, hid::Action)>
  getButtonFactory(const engine::world::World& world, const std::string& controllerLayoutName)
{
  return [controllerLayoutName, &world](const engine::InputMappingConfig& gameMappingConfig,
                                        const engine::InputMappingConfig& menuMappingConfig,
                                        hid::Action action) -> std::shared_ptr<ui::widgets::Widget>
  {
    auto gameKeys = getKeys(gameMappingConfig, engine::NamedAction{action});
    auto menuKeys = getKeys(menuMappingConfig, engine::NamedAction{action});

    auto keys = gameKeys;
    keys.insert(keys.end(), menuKeys.cbegin(), menuKeys.cend());

    if(keys.empty())
    {
      return std::make_shared<ui::widgets::Label>(
        /* translators: TR charcmap encoding */ pgettext("ButtonAssignment", "N/A"));
    }

    gsl_Assert(keys.size() == 1);
    if(std::holds_alternative<engine::NamedGlfwKey>(keys[0]))
    {
      return std::make_shared<ui::widgets::Label>(hid::getName(std::get<engine::NamedGlfwKey>(keys[0])));
    }
    else if(std::holds_alternative<engine::NamedGlfwGamepadButton>(keys[0]))
    {
      gsl_Assert(!controllerLayoutName.empty());
      const auto& buttonMap = world.getWorldGeometry().getControllerLayouts().at(controllerLayoutName);
      const auto it = buttonMap.find(std::get<engine::NamedGlfwGamepadButton>(keys[0]).value);
      if(it == buttonMap.end())
        return std::make_shared<ui::widgets::Label>(
          /* translators: TR charcmap encoding */ pgettext("ButtonAssignment", "N/A"));

      return std::make_shared<ui::widgets::Sprite>(it->second);
    }
    else if(std::holds_alternative<engine::NamedAxisDir>(keys[0]))
    {
      gsl_Assert(!controllerLayoutName.empty());
      const auto& buttonMap = world.getWorldGeometry().getControllerLayouts().at(controllerLayoutName);
      const auto ref = std::get<engine::NamedAxisDir>(keys[0]);
      const auto it = buttonMap.find(ref.first.value);
      if(it == buttonMap.end())
        return std::make_shared<ui::widgets::Label>(
          /* translators: TR charcmap encoding */ pgettext("ButtonAssignment", "N/A"));

      const auto makeDirName = [&ref](const std::string& negativeName, const std::string& positiveName)
      {
        switch(ref.second.value)
        {
        case hid::GlfwAxisDir::Positive:
          return positiveName;
        case hid::GlfwAxisDir::Negative:
          return negativeName;
        default:
          BOOST_THROW_EXCEPTION(std::domain_error("axis direction"));
        }
      };

      std::string axisName;
      switch(ref.first.value)
      {
      case hid::GlfwAxis::LeftX:
        [[fallthrough]];
      case hid::GlfwAxis::RightX:
        axisName = makeDirName(/* translators: TR charcmap encoding */ pgettext("GamepadAxis", "Left"),
                               /* translators: TR charcmap encoding */ pgettext("GamepadAxis", "Right"));
        break;
      case hid::GlfwAxis::LeftY:
        [[fallthrough]];
      case hid::GlfwAxis::RightY:
        axisName = makeDirName(/* translators: TR charcmap encoding */ pgettext("GamepadAxis", "Up"),
                               /* translators: TR charcmap encoding */ pgettext("GamepadAxis", "Down"));
        break;
      case hid::GlfwAxis::LeftTrigger:
        [[fallthrough]];
      case hid::GlfwAxis::RightTrigger:
        break;
      }

      auto grid = std::make_shared<ui::widgets::GridBox>(glm::ivec2{0, 0});
      grid->setExtents(2, 1);
      auto s = std::make_shared<ui::widgets::Sprite>(it->second);
      s->fitToContent();
      grid->set(0, 0, s);
      auto l = std::make_shared<ui::widgets::Label>(axisName);
      l->fitToContent();
      grid->set(1, 0, l);
      grid->fitToContent();
      return grid;
    }
    else
    {
      BOOST_THROW_EXCEPTION(std::runtime_error("failed to create display widget for button assignment"));
    }
  };
}
} // namespace

ControlsMenuState::ControlsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                     std::unique_ptr<MenuState> previous,
                                     const engine::world::World& world)
    : SelectedMenuState{ringTransform}
    , m_previous{std::move(previous)}
    , m_editing{world.getPresenter().getInputHandler().getMappings()}
    , m_layout{std::make_shared<ui::widgets::GridBox>()}
    , m_controls{std::make_shared<ControlsWidget>(m_editing.at(0))}
    , m_resetKey{hid::GlfwKey::Backspace, 5}
    , m_deleteKey{hid::GlfwKey::Delete, 2}
    , m_confirm{std::make_shared<ui::widgets::SelectionBox>(
        /* translators: TR charmap encoding */ _("Apply changes?"),
        std::vector<std::string>{/* translators: TR charmap encoding */ _("Yes"),
                                 /* translators: TR charmap encoding */ _("No"),
                                 /* translators: TR charmap encoding */ _("Return")})}
    , m_error{std::make_shared<ui::widgets::SelectionBox>(
        ui::breakLines(/* translators: TR charmap encoding */ _("Ensure your menu mapping is fully configured."), 500),
        std::vector<std::string>{/* translators: TR charmap encoding */ _("OK"),
                                 /* translators: TR charmap encoding */ _("Discard changes")})}
{
  m_controls->updateBindings(m_editing.at(0), getButtonFactory(world, m_editing.at(0).controllerType));

  m_layout->setExtents(1, 6);

  m_layout->set(0, 0, m_controls);
  m_layout->set(0,
                1,
                std::make_shared<ui::widgets::Label>(
                  /* translators: TR charmap encoding */ _(
                    "Switch Profile: %1% and %2%", hid::getName(PrevProfileAction), hid::getName(NextProfileAction))));
  m_layout->set(0,
                2,
                std::make_shared<ui::widgets::Label>(
                  /* translators: TR charmap encoding */ _("Edit Mapping: %1%", hid::getName(EditAction))));
  m_layout->set(
    0,
    3,
    std::make_shared<ui::widgets::Label>(
      /* translators: TR charmap encoding */ _("Change Button Images: %1%", hid::getName(ChangeControllerTypeAction))));
  m_layout->set(0,
                4,
                std::make_shared<ui::widgets::Label>(
                  /* translators: TR charmap encoding */ _("Remove Mapping: Hold %1% for %2% seconds",
                                                           hid::getName(m_deleteKey.getKey()),
                                                           m_deleteKey.getDelay().count())));
  m_layout->set(0,
                5,
                std::make_shared<ui::widgets::Label>(
                  /* translators: TR charmap encoding */ _("Reset All Mappings: Hold %1% for %2% seconds",
                                                           hid::getName(m_resetKey.getKey()),
                                                           m_resetKey.getDelay().count())));
  m_confirm->fitToContent();
  m_error->fitToContent();
}

std::unique_ptr<MenuState> ControlsMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& /*display*/)
{
  if(m_mode == Mode::Display && world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Return))
  {
    if(m_editing == world.getEngine().getEngineConfig()->inputMappings)
    {
      m_mode = Mode::Discard;
    }
    else
    {
      bool validMapping = true;
      for(const auto& profiles : m_editing)
      {
        for(const auto required : {hid::Action::MenuLeft,
                                   hid::Action::MenuRight,
                                   hid::Action::MenuUp,
                                   hid::Action::MenuDown,
                                   hid::Action::PrevScreen,
                                   hid::Action::NextScreen,
                                   hid::Action::PrimaryInteraction,
                                   hid::Action::SecondaryInteraction,
                                   hid::Action::Return})
        {
          validMapping &= !getKeys(profiles.menuMappings, engine::NamedAction{required}).empty();
        }
      }
      if(validMapping)
        m_mode = Mode::ConfirmApply;
      else
        m_mode = Mode::Error;
    }
  }

  switch(m_mode)
  {
  case Mode::Display:
    handleDisplayInput(world);
    break;
  case Mode::ChangeKey:
    handleChangeKeyInput(world);
    break;
  case Mode::ConfirmApply:
    handleConfirmInput(world);
    break;
  case Mode::Apply:
    world.getEngine().getEngineConfig()->inputMappings = m_editing;
    world.getEngine().getPresenter().getInputHandler().setMappings(m_editing);
    return std::move(m_previous);
  case Mode::Discard:
    return std::move(m_previous);
  case Mode::Error:
    handleErrorInput(world);
    break;
  }

  m_controls->fitToContent();
  m_layout->fitToContent();

  m_layout->setPosition({(ui.getSize().x - m_layout->getSize().x) / 2, ui.getSize().y - 90 - m_layout->getSize().y});
  switch(m_mode)
  {
  case Mode::Display:
    m_layout->update(true);
    m_layout->draw(ui, world.getPresenter());
    break;
  case Mode::ChangeKey:
    m_layout->update(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now())
                         .time_since_epoch()
                         .count()
                       % BlinkPeriod.count()
                     < BlinkPeriod.count() / 2);
    m_layout->draw(ui, world.getPresenter());
    break;
  case Mode::ConfirmApply:
    m_confirm->update(true);
    m_confirm->setPosition(
      {(ui.getSize().x - m_confirm->getSize().x) / 2, ui.getSize().y - 200 - m_confirm->getSize().y});
    m_layout->draw(ui, world.getPresenter());
    m_confirm->draw(ui, world.getPresenter());
    break;
  case Mode::Error:
    m_error->update(true);
    m_error->setPosition({(ui.getSize().x - m_error->getSize().x) / 2, ui.getSize().y - 200 - m_error->getSize().y});
    m_layout->draw(ui, world.getPresenter());
    m_error->draw(ui, world.getPresenter());
    break;
  case Mode::Apply:
    [[fallthrough]];
  case Mode::Discard:
    break;
  }

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
    auto& editing = m_editing.at(m_editingIndex);
    m_controls->updateBindings(editing, getButtonFactory(world, editing.controllerType));
  }

  if(m_deleteKey.update(world.getPresenter().getInputHandler()))
  {
    auto& editing = m_editing.at(m_editingIndex);
    auto& inputMappings = m_controls->isMenuControlsSelected() ? editing.menuMappings : editing.gameMappings;
    auto inputs = getKeys(inputMappings, engine::NamedAction{m_controls->getCurrentAction()});
    for(auto input : inputs)
      inputMappings.erase(input);
    m_controls->updateBindings(editing, getButtonFactory(world, editing.controllerType));
  }

  if(world.getPresenter().getInputHandler().hasDebouncedAction(PrevProfileAction))
  {
    if(m_editingIndex == 0)
      m_editingIndex = m_editing.size() - 1;
    else
      --m_editingIndex;
    auto& editing = m_editing.at(m_editingIndex);
    m_controls->updateBindings(editing, getButtonFactory(world, editing.controllerType));
  }

  if(world.getPresenter().getInputHandler().hasDebouncedAction(NextProfileAction))
  {
    if(m_editingIndex == m_editing.size() - 1)
      m_editingIndex = 0;
    else
      ++m_editingIndex;
    auto& editing = m_editing.at(m_editingIndex);
    m_controls->updateBindings(editing, getButtonFactory(world, editing.controllerType));
  }

  if(world.getPresenter().getInputHandler().hasDebouncedAction(ChangeControllerTypeAction))
  {
    auto& layouts = world.getWorldGeometry().getControllerLayouts();
    auto& editing = m_editing.at(m_editingIndex);
    auto layoutIt = layouts.find(editing.controllerType);
    gsl_Assert(layoutIt != layouts.end());
    ++layoutIt;
    if(layoutIt == layouts.end())
      editing.controllerType = layouts.begin()->first;
    else
      editing.controllerType = layoutIt->first;
    m_controls->updateBindings(editing, getButtonFactory(world, editing.controllerType));
  }

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::MenuDown))
  {
    m_controls->nextRow();
  }
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::MenuUp))
  {
    m_controls->prevRow();
  }
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::MenuRight))
  {
    m_controls->nextColumn();
  }
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::MenuLeft))
  {
    m_controls->prevColumn();
  }

  if(world.getPresenter().getInputHandler().hasDebouncedAction(EditAction))
  {
    m_mode = Mode::ChangeKey;
    (void)world.getPresenter().getInputHandler().takeRecentlyPressedKey();
    (void)world.getPresenter().getInputHandler().takeRecentlyPressedButton();
    (void)world.getPresenter().getInputHandler().takeRecentlyPressedAxis();
  }
}

void ControlsMenuState::handleChangeKeyInput(engine::world::World& world)
{
  auto setInput = [&](const auto& newInput)
  {
    // it's fine to assign the same input to the same action in different profiles;
    // otherwise, remove the mapped input from all profiles before assigning it to the new action
    auto mappingGroup = m_controls->isMenuControlsSelected() ? &engine::NamedInputMappingConfig::menuMappings
                                                             : &engine::NamedInputMappingConfig::gameMappings;

    BOOST_LOG_TRIVIAL(debug) << "setInput: isMenuControlsSelected=" << m_controls->isMenuControlsSelected();
    const bool hasDifferentBindings
      = std::any_of(m_editing.begin(),
                    m_editing.end(),
                    [this, &newInput, &mappingGroup](const engine::NamedInputMappingConfig& mapping)
                    {
                      auto it = (mapping.*mappingGroup).find(newInput);
                      return it != (mapping.*mappingGroup).end() && it->second != m_controls->getCurrentAction();
                    });
    BOOST_LOG_TRIVIAL(debug) << "setInput: hasDifferentBindings=" << hasDifferentBindings;
    if(hasDifferentBindings)
    {
      for(auto& mapping : m_editing)
      {
        (mapping.*mappingGroup).erase(newInput);
      }
    }

    auto& mapping = m_editing.at(m_editingIndex);
    auto boundInputs = getKeys((mapping.*mappingGroup), engine::NamedAction{m_controls->getCurrentAction()});
    for(auto boundInput : boundInputs)
      (mapping.*mappingGroup).erase(boundInput);
    (mapping.*mappingGroup)[newInput] = m_controls->getCurrentAction();
    m_mode = Mode::Display;
    m_controls->updateBindings(mapping, getButtonFactory(world, mapping.controllerType));
  };

  if(const auto newKey = world.getPresenter().getInputHandler().takeRecentlyPressedKey())
  {
    setInput(*newKey);
  }
  else if(const auto newButton = world.getPresenter().getInputHandler().takeRecentlyPressedButton())
  {
    setInput(*newButton);
  }
  else if(const auto newAxis = world.getPresenter().getInputHandler().takeRecentlyPressedAxis())
  {
    setInput(*newAxis);
  }
}

void ControlsMenuState::handleConfirmInput(engine::world::World& world)
{
  if(world.getPresenter().getInputHandler().getInputState().menuZMovement.justChangedTo(hid::AxisMovement::Backward))
  {
    m_confirm->next();
  }
  else if(world.getPresenter().getInputHandler().getInputState().menuZMovement.justChangedTo(
            hid::AxisMovement::Forward))
  {
    m_confirm->prev();
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::PrimaryInteraction))
  {
    switch(m_confirm->getSelected())
    {
    case 0:
      m_mode = Mode::Apply;
      break;
    case 1:
      m_mode = Mode::Discard;
      break;
    case 2:
      m_mode = Mode::Display;
      break;
    default:
      BOOST_THROW_EXCEPTION(std::domain_error("invalid selection"));
    }
  }
}

void ControlsMenuState::handleErrorInput(engine::world::World& world)
{
  if(world.getPresenter().getInputHandler().getInputState().menuZMovement.justChangedTo(hid::AxisMovement::Backward))
  {
    m_error->next();
  }
  else if(world.getPresenter().getInputHandler().getInputState().menuZMovement.justChangedTo(
            hid::AxisMovement::Forward))
  {
    m_error->prev();
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::PrimaryInteraction))
  {
    switch(m_error->getSelected())
    {
    case 0:
      m_mode = Mode::Display;
      break;
    case 1:
      m_mode = Mode::Discard;
      break;
    default:
      BOOST_THROW_EXCEPTION(std::domain_error("invalid selection"));
    }
  }
}
} // namespace menu
