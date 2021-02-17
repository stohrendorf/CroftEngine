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
boost::container::flat_set<GlfwKey> pressedKeys;
void keyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{
  auto typed = static_cast<GlfwKey>(key);
  switch(action)
  {
  case GLFW_PRESS: pressedKeys.emplace(typed); break;
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

  for(const auto& [action, key] : m_inputKeyMap)
  {
    bool gamepadButtonPressed = false;
    if(auto it = m_inputGamepadMap.find(action); m_controllerIndex >= 0 && it != m_inputGamepadMap.end())
      gamepadButtonPressed = gamepadState.buttons[static_cast<int>(it->second)];

    m_inputState.actions[action] = isKeyPressed(key) || gamepadButtonPressed;
  }

  m_inputState.setXAxisMovement(m_inputState.actions[Action::Left], m_inputState.actions[Action::Right]);
  m_inputState.setZAxisMovement(m_inputState.actions[Action::Backward], m_inputState.actions[Action::Forward]);
  m_inputState.setStepMovement(m_inputState.actions[Action::StepLeft], m_inputState.actions[Action::StepRight]);
}

void InputHandler::setMapping(const InputMapping& inputMapping)
{
  m_inputGamepadMap.clear();
  m_inputKeyMap.clear();

  for(const auto& [action, inputs] : inputMapping)
  {
    for(const auto& input : inputs)
    {
      if(std::holds_alternative<GlfwGamepadButton>(input))
      {
        if(auto it = m_inputGamepadMap.find(action); it != m_inputGamepadMap.end())
        {
          BOOST_LOG_TRIVIAL(warning) << "Multiple gamepad mappings present for action " << toString(action);
        }
        m_inputGamepadMap[action] = std::get<GlfwGamepadButton>(input);
      }
      else if(std::holds_alternative<GlfwKey>(input))
      {
        if(auto it = m_inputKeyMap.find(action); it != m_inputKeyMap.end())
        {
          BOOST_LOG_TRIVIAL(warning) << "Multiple gamepad mappings present for action " << toString(action);
        }
        m_inputKeyMap[action] = std::get<GlfwKey>(input);
      }
      else
        BOOST_THROW_EXCEPTION(std::runtime_error("Invalid input type"));
    }
  }
}
} // namespace hid
