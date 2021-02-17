#pragma once

#include "inputstate.h"

#include <GLFW/glfw3.h>
#include <gsl-lite.hpp>
#include <variant>

namespace hid
{
enum class GlfwKey;
enum class GlfwGamepadButton;

using InputMapping = std::map<Action, std::vector<std::variant<GlfwGamepadButton, GlfwKey>>>;

class InputHandler final
{
public:
  explicit InputHandler(gsl::not_null<GLFWwindow*> window);
  void setMapping(const InputMapping& inputMapping);

  void update();

  [[nodiscard]] const InputState& getInputState() const
  {
    return m_inputState;
  }

  bool hasAction(Action action) const
  {
    if(auto it = m_inputState.actions.find(action); it != m_inputState.actions.end())
      return it->second.current;
    return false;
  }

  bool hasDebouncedAction(Action action) const
  {
    if(auto it = m_inputState.actions.find(action); it != m_inputState.actions.end())
      return it->second.justChangedTo(true);
    return false;
  }

private:
  InputState m_inputState{};
  const gsl::not_null<GLFWwindow*> m_window;
  int m_controllerIndex = -1;
  boost::container::flat_map<Action, GlfwKey> m_inputKeyMap{};
  boost::container::flat_map<Action, GlfwGamepadButton> m_inputGamepadMap{};
};
} // namespace hid
