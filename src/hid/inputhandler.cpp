#include "inputhandler.h"

#include "actions.h"
#include "axisdir.h"
#include "glfw_axes.h"
#include "glfw_axis_dirs.h"
#include "glfw_gamepad_buttons.h"
#include "glfw_keys.h"
#include "inputstate.h"
#include "serialization/named_enum.h"
#include "util/helpers.h"

#include <boost/container/flat_set.hpp>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <fstream>
#include <gl/glfw.h>
#include <gl/window.h>
#include <iterator>
#include <mutex>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace hid
{
namespace
{
// #define PRINT_KEY_INPUT

std::recursive_mutex glfwStateMutex;

boost::container::flat_set<int> connectedGamepads;
boost::container::flat_set<GlfwGamepadButton> pressedButtons;
std::optional<GlfwGamepadButton> recentPressedButton;

boost::container::flat_set<GlfwKey> pressedKeys;
std::optional<GlfwKey> recentPressedKey;

boost::container::flat_set<AxisDir> pressedAxes;
std::optional<AxisDir> recentPressedAxis;

constexpr float AxisDeadZone = 0.4f;

void keyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{
  if(key < 0)
    return;

  auto typed = static_cast<GlfwKey>(key);
  switch(action)
  {
  case GLFW_PRESS:
  {
    std::lock_guard lock{glfwStateMutex};
    pressedKeys.emplace(typed);
    recentPressedKey = typed;
#ifdef PRINT_KEY_INPUT
    if(const auto name = toString(typed))
      BOOST_LOG_TRIVIAL(debug) << "Key pressed: " << name;
    else
      BOOST_LOG_TRIVIAL(debug) << "Key pressed: Code " << key;
#endif
    break;
  }
  case GLFW_RELEASE:
  {
    std::lock_guard lock{glfwStateMutex};
    pressedKeys.erase(typed);
    break;
  }
  case GLFW_REPEAT:
    break;
  default:
    BOOST_THROW_EXCEPTION(std::domain_error("unexpected key action"));
  }
}

void joystickCallback(int jid, int event)
{
  switch(event)
  {
  case GLFW_CONNECTED:
    if(glfwJoystickIsGamepad(jid) == GLFW_FALSE)
    {
      BOOST_LOG_TRIVIAL(info) << "Connected joystick #" << glfwGetJoystickName(jid) << " is not a gamepad";
      break;
    }

    {
      std::lock_guard lock{glfwStateMutex};
      connectedGamepads.emplace(jid);
    }
    BOOST_LOG_TRIVIAL(info) << "Gamepad #" << jid << " connected: " << glfwGetGamepadName(jid);
    break;
  case GLFW_DISCONNECTED:
  {
    std::lock_guard lock{glfwStateMutex};
    if(connectedGamepads.erase(jid) != 0)
      BOOST_LOG_TRIVIAL(info) << "Gamepad #" << jid << " disconnected";
    break;
  }
  default:
    BOOST_THROW_EXCEPTION(std::domain_error("invalid joystick connection event"));
  }
}

void installHandlers(GLFWwindow* window)
{
  static bool installed = false;

  if(installed)
    return;

  glfwSetKeyCallback(window, &keyCallback);
  glfwSetJoystickCallback(joystickCallback);
  installed = true;
}

bool isKeyPressed(GlfwKey key)
{
  std::lock_guard lock{glfwStateMutex};
  return pressedKeys.count(key) > 0;
}
} // namespace

InputHandler::InputHandler(gslu::nn_shared<gl::Window> window, const std::filesystem::path& gameControllerDb)
    : m_window{std::move(window)}
{
  std::ifstream gameControllerDbFile{util::ensureFileExists(gameControllerDb), std::ios::in | std::ios::binary};
  std::noskipws(gameControllerDbFile);
  std::string gameControllerDbData{std::istream_iterator<char>{gameControllerDbFile}, std::istream_iterator<char>{}};

  gsl_Assert(glfwUpdateGamepadMappings(gameControllerDbData.c_str()) == GLFW_TRUE);

  installHandlers(m_window->getWindow());

  for(auto jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid)
  {
    if(glfwJoystickPresent(jid) != GLFW_TRUE)
      continue;

    const gsl::czstring name = glfwGetGamepadName(jid);
    if(name == nullptr || glfwJoystickIsGamepad(jid) != GLFW_TRUE)
    {
      BOOST_LOG_TRIVIAL(info) << "Connected joystick #" << jid << " " << glfwGetJoystickName(jid)
                              << " is not a gamepad";
      continue;
    }

    BOOST_LOG_TRIVIAL(info) << "Found gamepad controller #" << jid << ": " << name;

    std::lock_guard lock{glfwStateMutex};
    connectedGamepads.emplace(jid);
  }
}

void InputHandler::update()
{
  std::lock_guard lock{glfwStateMutex};
  if(!m_window->hasFocus())
  {
    for(auto& [action, button] : m_inputState.actions)
      button = false;
    m_inputState.setXAxisMovement(false, false);
    m_inputState.setMenuXAxisMovement(false, false);
    m_inputState.setZAxisMovement(false, false);
    m_inputState.setMenuZAxisMovement(false, false);
    m_inputState.setStepMovement(false, false);
    return;
  }

  std::vector<GLFWgamepadstate> gamepadStates;
  gamepadStates.reserve(connectedGamepads.size());
  for(auto jid : connectedGamepads)
  {
    if(glfwJoystickPresent(jid) != GLFW_TRUE)
      continue;

    GLFWgamepadstate state;
    gsl_Assert(glfwGetGamepadState(jid, &state) == GLFW_TRUE);
    gamepadStates.emplace_back(state);
  }

  const auto prevPressedButtons = std::exchange(pressedButtons, {});
  for(const auto& [button, _] : EnumUtil<GlfwGamepadButton>::all())
  {
    for(const auto& state : gamepadStates)
    {
      if(state.buttons[static_cast<int>(button)] == GLFW_RELEASE)
        continue;

      pressedButtons.emplace(button);
      if(prevPressedButtons.count(button) == 0)
        recentPressedButton = button;
      break;
    }
  }

  const auto prevPressedAxes = std::exchange(pressedAxes, {});
  for(const auto& [axis, _] : EnumUtil<GlfwAxis>::all())
  {
    for(const auto& state : gamepadStates)
    {
      if(const auto value = state.axes[static_cast<int>(axis)]; std::abs(value) > AxisDeadZone)
      {
        const AxisDir axisDir{axis, value > 0 ? GlfwAxisDir::Positive : GlfwAxisDir::Negative};
        pressedAxes.emplace(axisDir);
        if(prevPressedAxes.count(axisDir) == 0)
          recentPressedAxis = axisDir;
        break;
      }
    }
  }

  boost::container::flat_map<Action, bool> newActionStates{};
  newActionStates.reserve(m_mergedGameInputMappings.size() + m_mergedMenuInputMappings.size());

  for(const auto& [input, action] : m_mergedGameInputMappings)
  {
    auto& state = newActionStates[action.value];

    if(std::holds_alternative<engine::NamedGlfwGamepadButton>(input))
    {
      state |= pressedButtons.count(std::get<engine::NamedGlfwGamepadButton>(input).value) > 0;
    }
    else if(std::holds_alternative<engine::NamedGlfwKey>(input))
    {
      state |= isKeyPressed(std::get<engine::NamedGlfwKey>(input).value);
    }
    else
    {
      const auto mapped = std::get<engine::NamedAxisDir>(input);
      state |= pressedAxes.count({mapped.first.value, mapped.second.value}) > 0;
    }
  }

  for(const auto& [input, action] : m_mergedMenuInputMappings)
  {
    auto& state = newActionStates[action.value];

    if(std::holds_alternative<engine::NamedGlfwGamepadButton>(input))
    {
      state |= pressedButtons.count(std::get<engine::NamedGlfwGamepadButton>(input).value) > 0;
    }
    else if(std::holds_alternative<engine::NamedGlfwKey>(input))
    {
      state |= isKeyPressed(std::get<engine::NamedGlfwKey>(input).value);
    }
    else
    {
      const auto mapped = std::get<engine::NamedAxisDir>(input);
      state |= pressedAxes.count({mapped.first.value, mapped.second.value}) > 0;
    }
  }

  for(const auto& [action, state] : newActionStates)
  {
    m_inputState.actions[action] = state;
  }
  if(m_inputState.actions[Action::Backward] && m_inputState.actions[Action::Forward])
  {
    m_inputState.actions[Action::Roll] = true;
  }

  m_inputState.setXAxisMovement(m_inputState.actions[Action::Left], m_inputState.actions[Action::Right]);
  m_inputState.setMenuXAxisMovement(m_inputState.actions[Action::MenuLeft], m_inputState.actions[Action::MenuRight]);
  m_inputState.setZAxisMovement(m_inputState.actions[Action::Backward], m_inputState.actions[Action::Forward]);
  m_inputState.setMenuZAxisMovement(m_inputState.actions[Action::MenuDown], m_inputState.actions[Action::MenuUp]);
  m_inputState.setStepMovement(m_inputState.actions[Action::StepLeft], m_inputState.actions[Action::StepRight]);
}

void InputHandler::setMappings(const std::vector<engine::NamedInputMappingConfig>& inputMappings)
{
  m_inputMappings = inputMappings;

  m_mergedGameInputMappings.clear();
  for(const auto& mapping : m_inputMappings)
  {
    for(const auto& [input, action] : mapping.gameMappings)
    {
      gsl_Assert(m_mergedGameInputMappings.insert({input, action}).second);
    }
  }

  m_mergedMenuInputMappings.clear();
  for(const auto& mapping : m_inputMappings)
  {
    for(const auto& [input, action] : mapping.menuMappings)
    {
      gsl_Assert(m_mergedMenuInputMappings.insert({input, action}).second);
    }
  }
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
bool InputHandler::hasKey(GlfwKey key) const
{
  return isKeyPressed(key);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::optional<GlfwKey> InputHandler::takeRecentlyPressedKey()
{
  std::lock_guard lock{glfwStateMutex};
  return std::exchange(recentPressedKey, std::nullopt);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::optional<GlfwGamepadButton> InputHandler::takeRecentlyPressedButton()
{
  std::lock_guard lock{glfwStateMutex};
  return std::exchange(recentPressedButton, std::nullopt);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::optional<AxisDir> InputHandler::takeRecentlyPressedAxis()
{
  std::lock_guard lock{glfwStateMutex};
  return std::exchange(recentPressedAxis, std::nullopt);
}
} // namespace hid
