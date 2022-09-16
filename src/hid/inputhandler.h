#pragma once

#include "actions.h"
#include "axisdir.h"
#include "engine/engineconfig.h"
#include "inputstate.h"
#include "util/smallcollections.h"

#include <algorithm>
#include <boost/container/vector.hpp>
#include <filesystem>
#include <gl/glfw.h>
#include <gl/soglb_fwd.h>
#include <gsl/gsl-lite.hpp>
#include <map>
#include <optional>
#include <vector>

namespace hid
{
enum class GlfwKey;
enum class GlfwGamepadButton;

class InputHandler final
{
public:
  explicit InputHandler(gsl::not_null<std::shared_ptr<gl::Window>> window,
                        const std::filesystem::path& gameControllerDb);
  void setMappings(const std::vector<engine::NamedInputMappingConfig>& inputMappings);

  void update();

  [[nodiscard]] const InputState& getInputState() const
  {
    return m_inputState;
  }

  [[nodiscard]] bool hasAction(Action action) const
  {
    if(auto tmp = util::tryGet(m_inputState.actions, action); tmp.has_value())
      return tmp->get();

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
    if(auto tmp = util::tryGet(m_inputState.actions, action); tmp.has_value())
      return tmp->get().justChangedTo(true);
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
  const gsl::not_null<std::shared_ptr<gl::Window>> m_window;
  std::vector<engine::NamedInputMappingConfig> m_inputMappings{};
  engine::InputMappingConfig m_mergedInputMappings{};
};
} // namespace hid
