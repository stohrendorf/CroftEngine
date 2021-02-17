#include "inputhandler.h"

#include "glfw_keys.h"

#include <boost/log/trivial.hpp>
#include <unordered_set>
#include <utility>

namespace hid
{
namespace
{
std::unordered_set<GlfwKey> pressedKeys;
std::optional<GlfwKey> mostRecentPressedKey;
void keyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{
  auto typed = static_cast<GlfwKey>(key);
  switch(action)
  {
  case GLFW_PRESS:
    pressedKeys.emplace(typed);
    mostRecentPressedKey = typed;
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
} // namespace

bool isKeyPressed(GlfwKey key)
{
  return pressedKeys.count(key) > 0;
}

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

#define PS1_TRIANGLE GLFW_GAMEPAD_BUTTON_Y
#define PS1_SQUARE GLFW_GAMEPAD_BUTTON_X
#define PS1_CIRCLE GLFW_GAMEPAD_BUTTON_B
#define PS1_CROSS GLFW_GAMEPAD_BUTTON_A
#define PS1_L2 GLFW_GAMEPAD_AXIS_LEFT_TRIGGER
#define PS1_R2 GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER
#define PS1_L1 GLFW_GAMEPAD_BUTTON_LEFT_BUMPER
#define PS1_R1 GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER
#define PS1_HOME GLFW_GAMEPAD_BUTTON_START

void InputHandler::update()
{
  static constexpr float AxisDeadzone = 0.5f;

  GLFWgamepadstate gamepadState;
  if(m_controllerIndex >= 0)
    glfwGetGamepadState(m_controllerIndex, &gamepadState);

  auto left = isKeyPressed(GlfwKey::A);
  if(m_controllerIndex >= 0 && gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS)
    left = true;

  auto right = isKeyPressed(GlfwKey::D);
  if(m_controllerIndex >= 0 && gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] == GLFW_PRESS)
    right = true;

  auto stepLeft = isKeyPressed(GlfwKey::Q);
  if(m_controllerIndex >= 0 && gamepadState.axes[PS1_L2] > AxisDeadzone)
    stepLeft = true;

  auto stepRight = isKeyPressed(GlfwKey::E);
  if(m_controllerIndex >= 0 && gamepadState.axes[PS1_R2] > AxisDeadzone)
    stepRight = true;

  auto forward = isKeyPressed(GlfwKey::W);
  if(m_controllerIndex >= 0 && gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS)
    forward = true;

  auto backward = isKeyPressed(GlfwKey::S);
  if(m_controllerIndex >= 0 && gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS)
    backward = true;

  m_inputState.moveSlow = isKeyPressed(GlfwKey::LeftShift) || isKeyPressed(GlfwKey::RightShift)
                          || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_R1] == GLFW_PRESS);

  m_inputState.action = isKeyPressed(GlfwKey::LeftControl) || isKeyPressed(GlfwKey::RightControl)
                        || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_CROSS] == GLFW_PRESS);

  m_inputState.holster
    = isKeyPressed(GlfwKey::R) || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_TRIANGLE] == GLFW_PRESS);

  m_inputState.jump
    = isKeyPressed(GlfwKey::Space) || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_SQUARE] == GLFW_PRESS);

  m_inputState.roll
    = isKeyPressed(GlfwKey::X) || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_CIRCLE] == GLFW_PRESS);

  m_inputState.freeLook
    = isKeyPressed(GlfwKey::Kp0) || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_L1] == GLFW_PRESS);

  m_inputState.menu
    = isKeyPressed(GlfwKey::Escape) || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_HOME] == GLFW_PRESS);

  m_inputState.debug = isKeyPressed(GlfwKey::F11);
#ifndef NDEBUG
  m_inputState.cheatDive = isKeyPressed(GlfwKey::F10);
#endif

  m_inputState._1 = isKeyPressed(GlfwKey::Num1)
                    || (m_controllerIndex >= 0 && gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] < -AxisDeadzone);
  m_inputState._2 = isKeyPressed(GlfwKey::Num2)
                    || (m_controllerIndex >= 0 && gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] > AxisDeadzone);
  m_inputState._3 = isKeyPressed(GlfwKey::Num3)
                    || (m_controllerIndex >= 0 && gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] < -AxisDeadzone);
  m_inputState._4 = isKeyPressed(GlfwKey::Num4)
                    || (m_controllerIndex >= 0 && gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] > AxisDeadzone);
  m_inputState._5 = isKeyPressed(GlfwKey::Num5);
  m_inputState._6 = isKeyPressed(GlfwKey::Num6);

  m_inputState.save = isKeyPressed(GlfwKey::F5);
  m_inputState.load = isKeyPressed(GlfwKey::F6);

  double x, y;
  glfwGetCursorPos(m_window, &x, &y);

  if(m_controllerIndex >= 0)
  {
    if(m_inputState.freeLook)
    {
      x += gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_X] * 500;
      y -= gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] * 500;
    }
    else
    {
      if(gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] < -AxisDeadzone)
        forward = true;
      if(gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] > AxisDeadzone)
        backward = true;
      if(gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_X] < -AxisDeadzone)
        left = true;
      if(gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_X] > AxisDeadzone)
        right = true;
    }
  }

  m_inputState.setXAxisMovement(left, right);
  m_inputState.setZAxisMovement(backward, forward);
  m_inputState.setStepMovement(stepLeft, stepRight);
}
} // namespace hid
