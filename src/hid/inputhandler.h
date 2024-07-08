#pragma once

#include "actions.h"
#include "axisdir.h"
#include "engine/engineconfig.h"
#include "inputstate.h"

#include <algorithm>
#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>
#include <filesystem>
#include <gl/soglb_fwd.h>
#include <gslu.h>
#include <optional>
#include <utility>
#include <vector>

namespace hid
{
enum class GlfwKey;
enum class GlfwGamepadButton;

class InputHandler final
{
public:
  explicit InputHandler(gslu::nn_shared<gl::Window> window, const std::filesystem::path& gameControllerDb);
  ~InputHandler();
  void setMappings(const std::vector<engine::NamedInputMappingConfig>& inputMappings);

  void update();

  [[nodiscard]] const InputState& getInputState() const
  {
    return m_inputState;
  }

  [[nodiscard]] bool hasAction(Action action) const
  {
    if(auto tmp = m_inputState.actions.find(action); tmp != m_inputState.actions.end())
      return tmp->second;

    return false;
  }

  [[nodiscard]] bool hasAnyAction() const
  {
    return std::any_of(m_inputState.actions.begin(),
                       m_inputState.actions.end(),
                       [](const std::pair<Action, InputState::Button>& action)
                       {
                         return action.second.current;
                       });
  }

  [[nodiscard]] bool hasDebouncedAction(Action action) const
  {
    if(auto tmp = m_inputState.actions.find(action); tmp != m_inputState.actions.end())
      return tmp->second.justChangedTo(true);
    return false;
  }

  [[nodiscard]] const auto& getMappings() const
  {
    return m_inputMappings;
  }

  [[nodiscard]] bool hasKey(GlfwKey key) const;

  [[nodiscard]] std::optional<GlfwKey> takeRecentlyPressedKey();
  [[nodiscard]] std::optional<GlfwGamepadButton> takeRecentlyPressedButton();
  [[nodiscard]] std::optional<AxisDir> takeRecentlyPressedAxis();

private:
  InputState m_inputState{};
  gslu::nn_shared<gl::Window> m_window;
  std::vector<engine::NamedInputMappingConfig> m_inputMappings;
  engine::InputMappingConfig m_mergedGameInputMappings;
  engine::InputMappingConfig m_mergedMenuInputMappings;
};
} // namespace hid
