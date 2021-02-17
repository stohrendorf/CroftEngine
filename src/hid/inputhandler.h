#pragma once

#include "inputstate.h"

#include <GLFW/glfw3.h>
#include <gsl-lite.hpp>

namespace hid
{
enum class GlfwKey;

extern bool isKeyPressed(GlfwKey key);

class InputHandler final
{
public:
  explicit InputHandler(gsl::not_null<GLFWwindow*> window);

  void update();

  [[nodiscard]] const InputState& getInputState() const
  {
    return m_inputState;
  }

private:
  InputState m_inputState{};

  const gsl::not_null<GLFWwindow*> m_window;

  int m_controllerIndex = -1;
};
} // namespace hid
