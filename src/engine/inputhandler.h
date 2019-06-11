#pragma once

#include "inputstate.h"

#include "controller.h"

#include "gsl-lite.hpp"

namespace engine
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

    std::map<std::string, Controller> m_controllers;
    Controller* m_controller = nullptr;
    int m_controllerIndex = -1;
};
}
