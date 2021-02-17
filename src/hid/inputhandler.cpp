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
    , m_inputKeyMap{
        {Action::MoveSlow, GlfwKey::LeftShift},
        {Action::Action, GlfwKey::LeftControl},
        {Action::Holster, GlfwKey::R},
        {Action::Jump, GlfwKey::Space},
        {Action::Roll, GlfwKey::X},
        {Action::FreeLook, GlfwKey::Kp0},
        {Action::Menu, GlfwKey::Escape},
        {Action::Debug, GlfwKey::F11},
        {Action::DrawPistols, GlfwKey::Num1},
        {Action::DrawShotgun, GlfwKey::Num2},
        {Action::DrawUzis, GlfwKey::Num3},
        {Action::DrawMagnums, GlfwKey::Num4},
        {Action::ConsumeSmallMedipack, GlfwKey::Num5},
        {Action::ConsumeLargeMedipack, GlfwKey::Num6},
        {Action::Save, GlfwKey::F5},
        {Action::Load, GlfwKey::F6},
        {Action::Left, GlfwKey::A},
        {Action::Right, GlfwKey::D},
        {Action::Forward, GlfwKey::W},
        {Action::Backward, GlfwKey::S},
        {Action::StepLeft, GlfwKey::Q},
        {Action::StepRight, GlfwKey::E},
#ifndef NDEBUG
        {Action::CheatDive, GlfwKey::F10},
#endif
      },
    m_inputGamepadMap{
      {Action::MoveSlow, GlfwGamepadButton::RightBumper},
      {Action::Action, GlfwGamepadButton::A},
      {Action::Holster, GlfwGamepadButton::Y},
      {Action::Jump, GlfwGamepadButton::X},
      {Action::Roll, GlfwGamepadButton::B},
      {Action::FreeLook, GlfwGamepadButton::LeftBumper},
      {Action::Menu, GlfwGamepadButton::Start},
      {Action::Left, GlfwGamepadButton::DPadLeft},
      {Action::Right, GlfwGamepadButton::DPadRight},
      {Action::Forward, GlfwGamepadButton::DPadUp},
      {Action::Backward, GlfwGamepadButton::DPadDown},
    }
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
} // namespace hid
