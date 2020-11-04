#include "inputhandler.h"

#include <boost/algorithm/string.hpp>
#include <unordered_set>
#include <utility>

namespace hid
{
namespace
{
std::unordered_set<int> pressedKeys;
std::optional<int> mostRecentPressedKey;
void keyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{
  switch(action)
  {
  case GLFW_PRESS:
    pressedKeys.emplace(key);
    mostRecentPressedKey = key;
    break;
  case GLFW_RELEASE: pressedKeys.erase(key); break;
  case GLFW_REPEAT: break;
  default: Expects(false);
  }
}

std::unordered_set<int> pressedMouseButtons;
void mouseButtonCallback(GLFWwindow* /*window*/, int button, int action, int /*mods*/)
{
  switch(action)
  {
  case GLFW_PRESS: pressedMouseButtons.emplace(button); break;
  case GLFW_RELEASE: pressedMouseButtons.erase(button); break;
  default: Expects(false);
  }
}

void installHandlers(GLFWwindow* window)
{
  static bool installed = false;

  if(installed)
    return;

  glfwSetKeyCallback(window, &keyCallback);
  glfwSetMouseButtonCallback(window, &mouseButtonCallback);
  installed = true;
}
} // namespace

bool isKeyPressed(int key)
{
  return pressedKeys.count(key) > 0;
}

bool isMouseButtonPressed(int button)
{
  return pressedMouseButtons.count(button) > 0;
}

InputHandler::InputHandler(gsl::not_null<GLFWwindow*> window)
    : m_window{std::move(window)}
{
  glfwGetCursorPos(m_window, &m_lastCursorX, &m_lastCursorY);
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

  auto left = isKeyPressed(GLFW_KEY_A);
  if(m_controllerIndex >= 0 && gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS)
    left = true;

  auto right = isKeyPressed(GLFW_KEY_D);
  if(m_controllerIndex >= 0 && gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] == GLFW_PRESS)
    right = true;

  auto stepLeft = isKeyPressed(GLFW_KEY_Q);
  if(m_controllerIndex >= 0 && gamepadState.axes[PS1_L2] > AxisDeadzone)
    stepLeft = true;

  auto stepRight = isKeyPressed(GLFW_KEY_E);
  if(m_controllerIndex >= 0 && gamepadState.axes[PS1_R2] > AxisDeadzone)
    stepRight = true;

  auto forward = isKeyPressed(GLFW_KEY_W);
  if(m_controllerIndex >= 0 && gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS)
    forward = true;

  auto backward = isKeyPressed(GLFW_KEY_S);
  if(m_controllerIndex >= 0 && gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS)
    backward = true;

  m_inputState.moveSlow = isKeyPressed(GLFW_KEY_LEFT_SHIFT) || isKeyPressed(GLFW_KEY_RIGHT_SHIFT)
                          || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_R1] == GLFW_PRESS);

  m_inputState.action = isKeyPressed(GLFW_KEY_LEFT_CONTROL) || isKeyPressed(GLFW_KEY_RIGHT_CONTROL)
                        || isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)
                        || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_CROSS] == GLFW_PRESS);

  m_inputState.holster = isKeyPressed(GLFW_KEY_R) || isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)
                         || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_TRIANGLE] == GLFW_PRESS);

  m_inputState.jump
    = isKeyPressed(GLFW_KEY_SPACE) || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_SQUARE] == GLFW_PRESS);

  m_inputState.roll
    = isKeyPressed(GLFW_KEY_X) || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_CIRCLE] == GLFW_PRESS);

  m_inputState.freeLook = isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)
                          || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_L1] == GLFW_PRESS);

  m_inputState.menu
    = isKeyPressed(GLFW_KEY_ESCAPE) || (m_controllerIndex >= 0 && gamepadState.buttons[PS1_HOME] == GLFW_PRESS);

  m_inputState.debug = isKeyPressed(GLFW_KEY_F11);
  m_inputState.crt = isKeyPressed(GLFW_KEY_F2);

  m_inputState._1 = isKeyPressed(GLFW_KEY_1)
                    || (m_controllerIndex >= 0 && gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] < -AxisDeadzone);
  m_inputState._2 = isKeyPressed(GLFW_KEY_2)
                    || (m_controllerIndex >= 0 && gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] > AxisDeadzone);
  m_inputState._3 = isKeyPressed(GLFW_KEY_3)
                    || (m_controllerIndex >= 0 && gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] < -AxisDeadzone);
  m_inputState._4 = isKeyPressed(GLFW_KEY_4)
                    || (m_controllerIndex >= 0 && gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] > AxisDeadzone);
  m_inputState._5 = isKeyPressed(GLFW_KEY_5);
  m_inputState._6 = isKeyPressed(GLFW_KEY_6);

  m_inputState.save = isKeyPressed(GLFW_KEY_F5);
  m_inputState.load = isKeyPressed(GLFW_KEY_F6);

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

  m_inputState.mouseMovement = glm::vec2(x - std::exchange(m_lastCursorX, x), y - std::exchange(m_lastCursorY, y));

  m_inputState.setXAxisMovement(left, right);
  m_inputState.setZAxisMovement(backward, forward);
  m_inputState.setStepMovement(stepLeft, stepRight);
}
} // namespace hid
