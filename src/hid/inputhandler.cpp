#include "inputhandler.h"

#include "glfw_gamepad_buttons.h"
#include "glfw_keys.h"

#include <boost/container/flat_set.hpp>
#include <boost/log/trivial.hpp>
#include <utility>

namespace hid
{
namespace
{
#define PRINT_KEY_INPUT

boost::container::flat_set<GlfwKey> pressedKeys;
void keyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{
  auto typed = static_cast<GlfwKey>(key);
  switch(action)
  {
  case GLFW_PRESS: pressedKeys.emplace(typed);
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

void installHandlers(GLFWwindow* window)
{
  static bool installed = false;

  if(installed)
    return;

  glfwSetKeyCallback(window, &keyCallback);
  installed = true;
}

bool isKeyPressed(GlfwKey key)
{
  return pressedKeys.count(key) > 0;
}
} // namespace

InputHandler::InputHandler(gsl::not_null<GLFWwindow*> window)
    : m_window{std::move(window)}
{
  installHandlers(m_window);

  for(auto i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i)
  {
    if(glfwJoystickPresent(i) != GLFW_TRUE)
      continue;

    const gsl::czstring name = glfwGetGamepadName(i);
    if(name == nullptr)
      continue;

    if(!glfwJoystickIsGamepad(i))
      continue;

    BOOST_LOG_TRIVIAL(info) << "Found gamepad controller: " << name;
    m_controllerIndex = i;
    break;
  }
}

void InputHandler::update()
{
  GLFWgamepadstate gamepadState;
  if(m_controllerIndex >= 0)
    glfwGetGamepadState(m_controllerIndex, &gamepadState);

  boost::container::flat_map<Action, bool> states{};

  for(const auto& [input, action] : m_mergedInputMappings)
  {
    if(std::holds_alternative<engine::NamedGlfwGamepadButton>(input))
    {
      if(m_controllerIndex >= 0)
        states[action]
          |= gamepadState.buttons[static_cast<int>(std::get<engine::NamedGlfwGamepadButton>(input).value)] != 0;
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
} // namespace hid
