#pragma once

#include "inputstate.h"

#include "gsl-lite.hpp"
#include <GLFW/glfw3.h>

namespace hid
{
class InputHandler final
{
public:
    explicit InputHandler(const gsl::not_null<GLFWwindow*>& window);

    void update();

    const InputState& getInputState() const
    {
        return m_inputState;
    }

private:
    InputState m_inputState{};

    gsl::not_null<GLFWwindow*> m_window;
    double m_lastCursorX = 0;
    double m_lastCursorY = 0;

    int m_controllerIndex = -1;
};
}
