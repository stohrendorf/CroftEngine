#pragma once

#include "engine/engineconfig.h"
#include "inputstate.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <gsl/gsl-lite.hpp>
#include <variant>

namespace hid
{
enum class GlfwKey;
enum class GlfwGamepadButton;

class InputHandler final
{
public:
  explicit InputHandler(gsl::not_null<GLFWwindow*> window);
  void setMappings(const std::vector<engine::NamedInputMappingConfig>& inputMappings);

  void update();

  [[nodiscard]] const InputState& getInputState() const
  {
    return m_inputState;
  }

  [[nodiscard]] bool hasAction(Action action) const
  {
    if(auto it = m_inputState.actions.find(action); it != m_inputState.actions.end())
      return it->second.current;
    return false;
  }

  [[nodiscard]] bool hasAnyAction() const
  {
    return std::any_of(m_inputState.actions.begin(),
                       m_inputState.actions.end(),
                       [](const std::pair<Action, InputState::Button>& action) { return action.second.current; });
  }

  [[nodiscard]] bool hasDebouncedAction(Action action) const
  {
    if(auto it = m_inputState.actions.find(action); it != m_inputState.actions.end())
      return it->second.justChangedTo(true);
    return false;
  }

  [[nodiscard]] const auto& getMappings() const
  {
    return m_inputMappings;
  }

  [[nodiscard]] bool hasKey(GlfwKey key) const;

  [[nodiscard]] std::optional<GlfwKey> takeRecentlyPressedKey();

private:
  InputState m_inputState{};
  const gsl::not_null<GLFWwindow*> m_window;
  int m_controllerIndex = -1;
  std::vector<engine::NamedInputMappingConfig> m_inputMappings{};
  engine::InputMappingConfig m_mergedInputMappings{};
};
} // namespace hid
