#pragma once

#include "inputstate.h"

#include "controlleraxis.h"
#include "controllerbutton.h"

#include <gsl/gsl>
#include <GLFW/glfw3.h>


namespace engine
{
    class InputHandler final
    {
    public:
        explicit InputHandler(const gsl::not_null<GLFWwindow*>& window)
            : m_window(window)
        {
            glfwGetCursorPos(m_window, &m_lastCursorX, &m_lastCursorY);

            for( auto i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i )
            {
                if( glfwJoystickPresent(i) != GL_TRUE )
                    continue;

                const char* name = glfwGetJoystickName(i);
                if( name == nullptr )
                    continue;

                if( std::strcmp(name, "Xbox 360 Controller") != 0 && std::strcmp(name, "Wireless Xbox 360 Controller") != 0 )
                    continue;

                m_xboxController = i;
                break;
            }
        }


        void update()
        {
            // axes
            static const constexpr int LS_X = 0;
            static const constexpr int LS_Y = 1;
            static const constexpr int RS_X = 2;
            static const constexpr int RS_Y = 3;
            static const constexpr int LTRIGGER = 4;
            static const constexpr int RTRIGGER = 5;

            // buttons
            static const constexpr int A = 0;
            static const constexpr int B = 1;
            static const constexpr int X = 2;
            static const constexpr int Y = 3;
            static const constexpr int LB = 4;
            static const constexpr int RB = 5;
            static const constexpr int BACK = 6;
            static const constexpr int START = 7;
            static const constexpr int LS = 8;
            static const constexpr int RS = 9;
            static const constexpr int DPAD_UP = 10;
            static const constexpr int DPAD_RIGHT = 11;
            static const constexpr int DPAD_DOWN = 12;
            static const constexpr int DPAD_LEFT = 13;

            static const constexpr float AxisThreshold = 0.5f;

            int xboxButtonCount = 0;
            const uint8_t* xboxButtons = nullptr;
            int xboxAxisCount = 0;
            const float* xboxAxisValues = nullptr;

            auto isXboxButtonPressed = [&xboxButtonCount, &xboxButtons](int idx)
            {
                return xboxButtonCount > idx && xboxButtons != nullptr && xboxButtons[idx] == GLFW_PRESS;
            };

            auto getXboxAxis = [&xboxAxisCount, &xboxAxisValues](int idx, float def) -> float
            {
                if(xboxAxisCount <= idx || xboxAxisValues == nullptr)
                    return def;

                return xboxAxisValues[idx];
            };

            if(m_xboxController >= 0)
            {
                xboxButtons = glfwGetJoystickButtons(m_xboxController, &xboxButtonCount);
                xboxAxisValues = glfwGetJoystickAxes(m_xboxController, &xboxAxisCount);
            }

            auto left = glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS;
            if(isXboxButtonPressed(DPAD_LEFT))
                left = true;

            auto right = glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS;
            if(isXboxButtonPressed(DPAD_RIGHT))
                right = true;

            auto stepLeft = glfwGetKey(m_window, GLFW_KEY_Q) == GLFW_PRESS;
            if(isXboxButtonPressed(LB))
                stepLeft = true;

            auto stepRight = glfwGetKey(m_window, GLFW_KEY_E) == GLFW_PRESS;
            if(isXboxButtonPressed(RB))
                stepRight = true;

            auto forward = glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS;
            if(isXboxButtonPressed(DPAD_UP))
                forward = true;

            auto backward = glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS;
            if(isXboxButtonPressed(DPAD_DOWN))
                backward = true;

            m_inputState.moveSlow = glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
                                    || glfwGetKey(m_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
            if(isXboxButtonPressed(B))
                m_inputState.moveSlow = true;

            m_inputState.action = glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS
                                  || glfwGetKey(m_window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS
                                  || glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
            if(isXboxButtonPressed(X))
                m_inputState.action = true;

            m_inputState.jump = glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS;
            if(isXboxButtonPressed(A))
                m_inputState.jump = true;

            m_inputState.roll = glfwGetKey(m_window, GLFW_KEY_X) == GLFW_PRESS;
            if(isXboxButtonPressed(BACK))
                m_inputState.roll = true;

            m_inputState.freeLook = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
            if(isXboxButtonPressed(Y))
                m_inputState.freeLook = true;

            double x, y;
            glfwGetCursorPos(m_window, &x, &y);

            if(m_inputState.freeLook)
            {
                x += getXboxAxis(LS_X, 0) * 500;
                y += getXboxAxis(LS_Y, 0) * 500;
            }
            else
            {
                if(getXboxAxis(LS_Y, 0) > AxisThreshold)
                    forward = true;
                if(getXboxAxis(LS_Y, 0) < -AxisThreshold)
                    backward = true;
                if(getXboxAxis(LS_X, 0) < -AxisThreshold)
                    left = true;
                if(getXboxAxis(LS_X, 0) > AxisThreshold)
                    right = true;
            }

            m_inputState.mouseMovement = glm::vec2(x - m_lastCursorX, y - m_lastCursorY);
            m_lastCursorX = x;
            m_lastCursorY = y;

            m_inputState.setXAxisMovement(left, right);
            m_inputState.setZAxisMovement(backward, forward);
            m_inputState.setStepMovement(stepLeft, stepRight);
        }


        const InputState& getInputState() const
        {
            return m_inputState;
        }


    private:
        InputState m_inputState;

        gsl::not_null<GLFWwindow*> m_window;
        double m_lastCursorX = 0;
        double m_lastCursorY = 0;

        int m_xboxController = -1;
    };
}
