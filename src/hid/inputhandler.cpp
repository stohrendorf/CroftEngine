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
#include "util/smallcollections.h"

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

std::vector<int> connectedGamepads;
std::vector<GlfwGamepadButton> pressedButtons;
std::optional<GlfwGamepadButton> recentPressedButton;

std::vector<GlfwKey> pressedKeys;
std::optional<GlfwKey> recentPressedKey;

std::vector<AxisDir> pressedAxes;
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
    util::insertUnique(pressedKeys, typed);
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
    util::eraseUnique(pressedKeys, typed);
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
      util::insertUnique(connectedGamepads, jid);
    }
    BOOST_LOG_TRIVIAL(info) << "Gamepad #" << jid << " connected: " << glfwGetGamepadName(jid);
    break;
  case GLFW_DISCONNECTED:
  {
    std::lock_guard lock{glfwStateMutex};
    if(util::eraseUnique(connectedGamepads, jid))
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
  return util::containsUnique(pressedKeys, key);
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
    util::insertUnique(connectedGamepads, jid);
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
    m_inputState.setZAxisMovement(false, false);
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

      util::insertUnique(pressedButtons, button);
      if(!util::containsUnique(prevPressedButtons, button))
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
        util::insertUnique(pressedAxes, axisDir);
        if(!util::containsUnique(prevPressedAxes, axisDir))
          recentPressedAxis = axisDir;
        break;
      }
    }
  }

  std::vector<std::pair<Action, bool>> newActionStates{};
  newActionStates.reserve(m_mergedInputMappings.size());

  for(const auto& [input, action] : m_mergedInputMappings)
  {
    auto& state = util::getOrCreate(newActionStates, action.value);

    if(std::holds_alternative<engine::NamedGlfwGamepadButton>(input))
    {
      state |= util::containsUnique(pressedButtons, std::get<engine::NamedGlfwGamepadButton>(input).value);
    }
    else if(std::holds_alternative<engine::NamedGlfwKey>(input))
    {
      state |= isKeyPressed(std::get<engine::NamedGlfwKey>(input).value);
    }
    else
    {
      const auto mapped = std::get<engine::NamedAxisDir>(input);
      state |= util::containsUnique(pressedAxes, {mapped.first.value, mapped.second.value});
    }
  }

  for(const auto& [action, state] : newActionStates)
  {
    util::getOrCreate(m_inputState.actions, action) = state;
  }
  if(util::getOrCreate(m_inputState.actions, Action::Backward)
     && util::getOrCreate(m_inputState.actions, Action::Forward))
  {
    util::getOrCreate(m_inputState.actions, Action::Roll) = true;
  }

  m_inputState.setXAxisMovement(util::getOrCreate(m_inputState.actions, Action::Left),
                                util::getOrCreate(m_inputState.actions, Action::Right));
  m_inputState.setZAxisMovement(util::getOrCreate(m_inputState.actions, Action::Backward),
                                util::getOrCreate(m_inputState.actions, Action::Forward));
  m_inputState.setStepMovement(util::getOrCreate(m_inputState.actions, Action::StepLeft),
                               util::getOrCreate(m_inputState.actions, Action::StepRight));
}

void InputHandler::setMappings(const std::vector<engine::NamedInputMappingConfig>& inputMappings)
{
  m_inputMappings = inputMappings;
  m_mergedInputMappings.clear();

  for(const auto& mapping : m_inputMappings)
  {
    for(const auto& [input, action] : mapping.mappings)
    {
      gsl_Assert(m_mergedInputMappings.insert({input, action}).second);
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
