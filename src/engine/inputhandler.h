#pragma once

#include "inputstate.h"

#include "controllerlayout.h"

#include "gsl-lite.hpp"

namespace engine
{
class InputHandler final
{
public:
    explicit InputHandler(const gsl::not_null<GLFWwindow*>& window)
            : m_window{window}
            , m_controllerLayout{"3rdparty/controller-data/Xbox-14-6-Windows-1.json"}
    {
        glfwGetCursorPos( m_window, &m_lastCursorX, &m_lastCursorY );

        for( auto i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i )
        {
            if( glfwJoystickPresent( i ) != GL_TRUE )
                continue;

            const char* name = glfwGetJoystickName( i );
            if( name == nullptr )
                continue;

            if( m_controllerLayout.getNames().find( name ) == m_controllerLayout.getNames().end() )
                continue;

            m_controllerIndex = i;
            break;
        }
    }

    void update()
    {
        static const constexpr float AxisThreshold = 0.5f;

        gsl::index controllerButtonCount = 0;
        const uint8_t* controllerButtons = nullptr;
        gsl::index controllerAxisCount = 0;
        const float* controllerAxisValues = nullptr;

        if( m_controllerIndex >= 0 )
        {
            int tmp = 0;
            controllerButtons = glfwGetJoystickButtons( m_controllerIndex, &tmp );
            controllerButtonCount = std::exchange( tmp, 0 );
            controllerAxisValues = glfwGetJoystickAxes( m_controllerIndex, &tmp );
            controllerAxisCount = std::exchange( tmp, 0 );
        }

        auto left = glfwGetKey( m_window, GLFW_KEY_A ) == GLFW_PRESS;
        if( m_controllerLayout.isButtonPressed( ControllerButton::DPAD_LEFT,
                                                gsl::span<const uint8_t>{controllerButtons, controllerButtonCount} ) )
            left = true;

        auto right = glfwGetKey( m_window, GLFW_KEY_D ) == GLFW_PRESS;
        if( m_controllerLayout.isButtonPressed( ControllerButton::DPAD_RIGHT,
                                                gsl::span<const uint8_t>{controllerButtons, controllerButtonCount} ) )
            right = true;

        auto stepLeft = glfwGetKey( m_window, GLFW_KEY_Q ) == GLFW_PRESS;
        if( m_controllerLayout.isButtonPressed( ControllerButton::LB,
                                                gsl::span<const uint8_t>{controllerButtons, controllerButtonCount} ) )
            stepLeft = true;

        auto stepRight = glfwGetKey( m_window, GLFW_KEY_E ) == GLFW_PRESS;
        if( m_controllerLayout.isButtonPressed( ControllerButton::RB,
                                                gsl::span<const uint8_t>{controllerButtons, controllerButtonCount} ) )
            stepRight = true;

        auto forward = glfwGetKey( m_window, GLFW_KEY_W ) == GLFW_PRESS;
        if( m_controllerLayout.isButtonPressed( ControllerButton::DPAD_UP,
                                                gsl::span<const uint8_t>{controllerButtons, controllerButtonCount} ) )
            forward = true;

        auto backward = glfwGetKey( m_window, GLFW_KEY_S ) == GLFW_PRESS;
        if( m_controllerLayout.isButtonPressed( ControllerButton::DPAD_DOWN,
                                                gsl::span<const uint8_t>{controllerButtons, controllerButtonCount} ) )
            backward = true;

        m_inputState.moveSlow = glfwGetKey( m_window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS
                                || glfwGetKey( m_window, GLFW_KEY_RIGHT_SHIFT ) == GLFW_PRESS;
        if( m_controllerLayout.isButtonPressed( ControllerButton::B,
                                                gsl::span<const uint8_t>{controllerButtons, controllerButtonCount} ) )
            m_inputState.moveSlow = true;

        m_inputState.action = glfwGetKey( m_window, GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS
                              || glfwGetKey( m_window, GLFW_KEY_RIGHT_CONTROL ) == GLFW_PRESS
                              || glfwGetMouseButton( m_window, GLFW_MOUSE_BUTTON_LEFT ) == GLFW_PRESS;
        if( m_controllerLayout.isButtonPressed( ControllerButton::X,
                                                gsl::span<const uint8_t>{controllerButtons, controllerButtonCount} ) )
            m_inputState.action = true;

        // TODO also check gamepad
        m_inputState.holster = glfwGetKey( m_window, GLFW_KEY_R ) == GLFW_PRESS
                               || glfwGetMouseButton( m_window, GLFW_MOUSE_BUTTON_MIDDLE ) == GLFW_PRESS;

        m_inputState.jump = glfwGetKey( m_window, GLFW_KEY_SPACE ) == GLFW_PRESS;
        if( m_controllerLayout.isButtonPressed( ControllerButton::A,
                                                gsl::span<const uint8_t>{controllerButtons, controllerButtonCount} ) )
            m_inputState.jump = true;

        m_inputState.roll = glfwGetKey( m_window, GLFW_KEY_X ) == GLFW_PRESS;
        if( m_controllerLayout.isButtonPressed( ControllerButton::BACK,
                                                gsl::span<const uint8_t>{controllerButtons, controllerButtonCount} ) )
            m_inputState.roll = true;

        m_inputState.freeLook = glfwGetMouseButton( m_window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS;
        if( m_controllerLayout.isButtonPressed( ControllerButton::Y,
                                                gsl::span<const uint8_t>{controllerButtons, controllerButtonCount} ) )
            m_inputState.freeLook = true;

        m_inputState.debug = glfwGetKey( m_window, GLFW_KEY_F11 ) == GLFW_PRESS;

        m_inputState._1 = glfwGetKey( m_window, GLFW_KEY_1 ) == GLFW_PRESS;
        m_inputState._2 = glfwGetKey( m_window, GLFW_KEY_2 ) == GLFW_PRESS;
        m_inputState._3 = glfwGetKey( m_window, GLFW_KEY_3 ) == GLFW_PRESS;
        m_inputState._4 = glfwGetKey( m_window, GLFW_KEY_4 ) == GLFW_PRESS;
        m_inputState._5 = glfwGetKey( m_window, GLFW_KEY_5 ) == GLFW_PRESS;
        m_inputState._6 = glfwGetKey( m_window, GLFW_KEY_6 ) == GLFW_PRESS;

        m_inputState.save = glfwGetKey( m_window, GLFW_KEY_F5 ) == GLFW_PRESS;
        m_inputState.load = glfwGetKey( m_window, GLFW_KEY_F6 ) == GLFW_PRESS;

        double x, y;
        glfwGetCursorPos( m_window, &x, &y );

        if( m_inputState.freeLook )
        {
            x += m_controllerLayout.getAxisValue( ControllerAxis::LS_X,
                                                  gsl::span<const float>{controllerAxisValues, controllerAxisCount}, 0 )
                 * 500;
            y += m_controllerLayout.getAxisValue( ControllerAxis::LS_Y,
                                                  gsl::span<const float>{controllerAxisValues, controllerAxisCount}, 0 )
                 * 500;
        }
        else
        {
            if( m_controllerLayout.getAxisValue( ControllerAxis::LS_Y,
                                                 gsl::span<const float>{controllerAxisValues, controllerAxisCount}, 0 )
                > AxisThreshold )
                forward = true;
            if( m_controllerLayout.getAxisValue( ControllerAxis::LS_Y,
                                                 gsl::span<const float>{controllerAxisValues, controllerAxisCount}, 0 )
                < -AxisThreshold )
                backward = true;
            if( m_controllerLayout.getAxisValue( ControllerAxis::LS_X,
                                                 gsl::span<const float>{controllerAxisValues, controllerAxisCount}, 0 )
                < -AxisThreshold )
                left = true;
            if( m_controllerLayout.getAxisValue( ControllerAxis::LS_X,
                                                 gsl::span<const float>{controllerAxisValues, controllerAxisCount}, 0 )
                > AxisThreshold )
                right = true;
        }

        m_inputState.mouseMovement = glm::vec2( x - m_lastCursorX, y - m_lastCursorY );
        m_lastCursorX = x;
        m_lastCursorY = y;

        m_inputState.setXAxisMovement( left, right );
        m_inputState.setZAxisMovement( backward, forward );
        m_inputState.setStepMovement( stepLeft, stepRight );
    }

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
    ControllerLayout m_controllerLayout;
};
}
