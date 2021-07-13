#include "inputhandler.h"

#include "glfw_gamepad_buttons.h"
#include "glfw_keys.h"
#include "util/helpers.h"

#include <boost/container/flat_set.hpp>
#include <boost/log/trivial.hpp>
#include <fstream>
#include <utility>

namespace hid
{
namespace
{
#define PRINT_KEY_INPUT

boost::container::flat_set<int> connectedGamepads;
boost::container::flat_set<GlfwGamepadButton> pressedButtons;
std::optional<GlfwGamepadButton> recentPressedButton;

boost::container::flat_set<GlfwKey> pressedKeys;
std::optional<GlfwKey> recentPressedKey;

void keyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{
  if(key < 0)
    return;

  auto typed = static_cast<GlfwKey>(key);
  switch(action)
  {
  case GLFW_PRESS: pressedKeys.emplace(typed); recentPressedKey = typed;
#ifdef PRINT_KEY_INPUT
    if(const auto name = toString(typed))
      BOOST_LOG_TRIVIAL(debug) << "Key pressed: " << name;
    else
      BOOST_LOG_TRIVIAL(debug) << "Key pressed: Code " << key;
#endif
    break;
  case GLFW_RELEASE: pressedKeys.erase(typed); break;
  case GLFW_REPEAT: break;
  default: Expects(false);
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

    connectedGamepads.emplace(jid);
    BOOST_LOG_TRIVIAL(info) << "Gamepad #" << jid << " connected: " << glfwGetGamepadName(jid);
    break;
  case GLFW_DISCONNECTED:
    if(connectedGamepads.erase(jid) > 0)
      BOOST_LOG_TRIVIAL(info) << "Gamepad #" << jid << " disconnected";
    break;
  default: BOOST_THROW_EXCEPTION(std::domain_error("invalid joystick connection event"));
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
  return pressedKeys.count(key) > 0;
}
} // namespace

InputHandler::InputHandler(gsl::not_null<GLFWwindow*> window, const std::filesystem::path& gameControllerDb)
    : m_window{std::move(window)}
{
  std::ifstream gameControllerDbFile{util::ensureFileExists(gameControllerDb), std::ios::in | std::ios::binary};
  std::noskipws(gameControllerDbFile);
  std::string gameControllerDbData{std::istream_iterator<char>{gameControllerDbFile}, std::istream_iterator<char>{}};

  Expects(glfwUpdateGamepadMappings(gameControllerDbData.c_str()) == GLFW_TRUE);

  installHandlers(m_window);

  for(auto i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i)
  {
    if(glfwJoystickPresent(i) != GLFW_TRUE)
      continue;

    const gsl::czstring name = glfwGetGamepadName(i);
    if(name == nullptr || glfwJoystickIsGamepad(i) != GLFW_TRUE)
    {
      BOOST_LOG_TRIVIAL(info) << "Connected joystick #" << i << " " << glfwGetJoystickName(i) << " is not a gamepad";
      continue;
    }

    BOOST_LOG_TRIVIAL(info) << "Found gamepad controller #" << i << ": " << name;
    connectedGamepads.emplace(i);
  }
}

void InputHandler::update()
{
  std::vector<GLFWgamepadstate> gamepadStates;
  gamepadStates.resize(connectedGamepads.size());
  std::transform(connectedGamepads.begin(),
                 connectedGamepads.end(),
                 std::back_inserter(gamepadStates),
                 [](int jid)
                 {
                   GLFWgamepadstate state;
                   Expects(glfwGetGamepadState(jid, &state) == GLFW_TRUE);
                   return state;
                 });

  boost::container::flat_map<Action, bool> states{};

  const auto prevPressedButtons = std::exchange(pressedButtons, {});
  for(const auto& [button, _] : hid::EnumUtil<GlfwGamepadButton>::all())
  {
    for(const auto& state : gamepadStates)
    {
      if(state.buttons[static_cast<int>(button)] != 0)
      {
        pressedButtons.emplace(button);
        if(prevPressedButtons.count(button) == 0)
          recentPressedButton = button;
        break;
      }
    }
  }

  for(const auto& [input, action] : m_mergedInputMappings)
  {
    if(std::holds_alternative<engine::NamedGlfwGamepadButton>(input))
    {
      states[action] |= pressedButtons.count(std::get<engine::NamedGlfwGamepadButton>(input).value) != 0;
    }
    else
    {
      states[action] |= isKeyPressed(std::get<engine::NamedGlfwKey>(input).value);
    }
  }

  for(const auto& [action, state] : states)
  {
    auto it = states.find(action);
    m_inputState.actions[action] = it->second;
  }
  m_inputState.setXAxisMovement(m_inputState.actions[Action::Left], m_inputState.actions[Action::Right]);
  m_inputState.setZAxisMovement(m_inputState.actions[Action::Backward], m_inputState.actions[Action::Forward]);
  m_inputState.setStepMovement(m_inputState.actions[Action::StepLeft], m_inputState.actions[Action::StepRight]);
}

void InputHandler::setMappings(const std::vector<engine::NamedInputMappingConfig>& inputMappings)
{
  m_inputMappings = inputMappings;
  m_mergedInputMappings.clear();

  for(const auto& mapping : m_inputMappings)
  {
    for(const auto& [input, action] : mapping.mappings)
    {
      Expects(m_mergedInputMappings.insert({input, action}).second);
    }
  }
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
bool InputHandler::hasKey(GlfwKey key) const
{
  return isKeyPressed(key);
}

std::optional<GlfwKey> InputHandler::takeRecentlyPressedKey()
{
  return std::exchange(recentPressedKey, std::nullopt);
}

std::optional<GlfwGamepadButton> InputHandler::takeRecentlyPressedButton()
{
  return std::exchange(recentPressedButton, std::nullopt);
}
} // namespace hid
