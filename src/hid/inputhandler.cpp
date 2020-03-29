#include "inputhandler.h"

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

namespace hid
{
InputHandler::InputHandler(const gsl::not_null<GLFWwindow*>& window)
    : m_window{window}
{
  glfwGetCursorPos(m_window, &m_lastCursorX, &m_lastCursorY);

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

void InputHandler::update()
{
  static constexpr float AxisThreshold = 0.5f;

  GLFWgamepadstate gamepadState;
  if(m_controllerIndex >= 0)
    glfwGetGamepadState(m_controllerIndex, &gamepadState);

  auto left = glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS;
  if(m_controllerIndex >= 0 && gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS)
    left = true;

  auto right = glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS;
  if(m_controllerIndex >= 0 && gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] == GLFW_PRESS)
    right = true;

  auto stepLeft = glfwGetKey(m_window, GLFW_KEY_Q) == GLFW_PRESS;
  if(m_controllerIndex >= 0 && gamepadState.axes[PS1_L2] > AxisThreshold)
    stepLeft = true;

  auto stepRight = glfwGetKey(m_window, GLFW_KEY_E) == GLFW_PRESS;
  if(m_controllerIndex >= 0 && gamepadState.axes[PS1_R2] > AxisThreshold)
    stepRight = true;

  auto forward = glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS;
  if(m_controllerIndex >= 0 && gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS)
    forward = true;

  auto backward = glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS;
  if(m_controllerIndex >= 0 && gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS)
    backward = true;

  m_inputState.moveSlow = glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
                          || glfwGetKey(m_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS
                          || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_R1] == GLFW_PRESS);

  m_inputState.action = glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS
                        || glfwGetKey(m_window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS
                        || glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS
                        || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_CROSS] == GLFW_PRESS);

  m_inputState.holster = glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS
                         || glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS
                         || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_TRIANGLE] == GLFW_PRESS);

  m_inputState.jump = glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS
                      || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_SQUARE] == GLFW_PRESS);

  m_inputState.roll = glfwGetKey(m_window, GLFW_KEY_X) == GLFW_PRESS
                      || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_CIRCLE] == GLFW_PRESS);

  m_inputState.freeLook = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS
                          || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_L1] == GLFW_PRESS);

  m_inputState.debug = glfwGetKey(m_window, GLFW_KEY_F11) == GLFW_PRESS;
  m_inputState.crt = glfwGetKey(m_window, GLFW_KEY_F2) == GLFW_PRESS;

  m_inputState._1 = glfwGetKey(m_window, GLFW_KEY_1) == GLFW_PRESS
                    || (m_controllerIndex >= 0 && gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] < -AxisThreshold);
  m_inputState._2 = glfwGetKey(m_window, GLFW_KEY_2) == GLFW_PRESS
                    || (m_controllerIndex >= 0 && gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] > AxisThreshold);
  m_inputState._3 = glfwGetKey(m_window, GLFW_KEY_3) == GLFW_PRESS
                    || (m_controllerIndex >= 0 && gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] < -AxisThreshold);
  m_inputState._4 = glfwGetKey(m_window, GLFW_KEY_4) == GLFW_PRESS
                    || (m_controllerIndex >= 0 && gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] > AxisThreshold);
  m_inputState._5 = glfwGetKey(m_window, GLFW_KEY_5) == GLFW_PRESS;
  m_inputState._6 = glfwGetKey(m_window, GLFW_KEY_6) == GLFW_PRESS;

  m_inputState.save = glfwGetKey(m_window, GLFW_KEY_F5) == GLFW_PRESS;
  m_inputState.load = glfwGetKey(m_window, GLFW_KEY_F6) == GLFW_PRESS;

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
      if(gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] < -AxisThreshold)
        forward = true;
      if(gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] > AxisThreshold)
        backward = true;
      if(gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_X] < -AxisThreshold)
        left = true;
      if(gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_X] > AxisThreshold)
        right = true;
    }
  }

  m_inputState.mouseMovement = glm::vec2(x - std::exchange(m_lastCursorX, x), y - std::exchange(m_lastCursorY, y));

  m_inputState.setXAxisMovement(left, right);
  m_inputState.setZAxisMovement(backward, forward);
  m_inputState.setStepMovement(stepLeft, stepRight);
}
} // namespace hid
