#include "inputhandler.h"

#include <boost/algorithm/string.hpp>

#include <fstream>

namespace hid
{

InputHandler::InputHandler(const gsl::not_null<GLFWwindow*>& window)
        : m_window{window}
{
    glfwGetCursorPos( m_window, &m_lastCursorX, &m_lastCursorY );

    auto readControllerDb = [&](const std::string& filename) {
        std::ifstream gcdb{filename, std::ios::in};
        if( !gcdb.is_open() )
            return;

        BOOST_LOG_TRIVIAL( info ) << "Loading controller mapping file " << filename;
        std::string line;
        while( std::getline( gcdb, line ) )
        {
            if( line.empty() || line[0] == '#' )
                continue;

            std::vector<std::string> tmp;
            boost::algorithm::split( tmp, line, boost::is_any_of( "," ) );
            Controller x{tmp};
            m_controllers.emplace( boost::algorithm::to_lower_copy( x.getGuid() ), x );
        }
    };

    readControllerDb( "gamecontrollerdb.txt" );
    readControllerDb( "glfwgamepaddb.txt" );

    for( auto i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i )
    {
        if( glfwJoystickPresent( i ) != GLFW_TRUE )
            continue;

        const char* name = glfwGetJoystickName( i );
        const char* guid = glfwGetJoystickGUID( i );
        if( name == nullptr || guid == nullptr )
            continue;

        BOOST_LOG_TRIVIAL( info ) << "Found joystick or gamepad controller: " << name << " (GUID " << guid << ")";

        const auto it = m_controllers.find( boost::algorithm::to_lower_copy( std::string( guid ) ) );
        if( it == m_controllers.end() )
        {
            BOOST_LOG_TRIVIAL( info ) << "No controller mapping found";
            continue;
        }

        BOOST_LOG_TRIVIAL( info ) << "Found controller mapping";
        m_controller = &it->second;
        m_controller->init();
        m_controllerIndex = i;
        break;
    }
}

void InputHandler::update()
{
    static const constexpr float AxisThreshold = 0.5f;

    if( m_controller != nullptr )
        m_controller->update( m_controllerIndex );

    auto left = glfwGetKey( m_window, GLFW_KEY_A ) == GLFW_PRESS;
    if( m_controller != nullptr && m_controller->isButtonPressed( ControllerButton::dpleft ) )
        left = true;

    auto right = glfwGetKey( m_window, GLFW_KEY_D ) == GLFW_PRESS;
    if( m_controller != nullptr && m_controller->isButtonPressed( ControllerButton::dpright ) )
        right = true;

    auto stepLeft = glfwGetKey( m_window, GLFW_KEY_Q ) == GLFW_PRESS;
    if( m_controller != nullptr && m_controller->isButtonPressed( ControllerButton::leftshoulder ) )
        stepLeft = true;

    auto stepRight = glfwGetKey( m_window, GLFW_KEY_E ) == GLFW_PRESS;
    if( m_controller != nullptr && m_controller->isButtonPressed( ControllerButton::rightshoulder ) )
        stepRight = true;

    auto forward = glfwGetKey( m_window, GLFW_KEY_W ) == GLFW_PRESS;
    if( m_controller != nullptr && m_controller->isButtonPressed( ControllerButton::dpup ) )
        forward = true;

    auto backward = glfwGetKey( m_window, GLFW_KEY_S ) == GLFW_PRESS;
    if( m_controller != nullptr && m_controller->isButtonPressed( ControllerButton::dpdown ) )
        backward = true;

    m_inputState.moveSlow = glfwGetKey( m_window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS
                            || glfwGetKey( m_window, GLFW_KEY_RIGHT_SHIFT ) == GLFW_PRESS;
    if( m_controller != nullptr && m_controller->isButtonPressed( ControllerButton::b ) )
        m_inputState.moveSlow = true;

    m_inputState.action = glfwGetKey( m_window, GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS
                          || glfwGetKey( m_window, GLFW_KEY_RIGHT_CONTROL ) == GLFW_PRESS
                          || glfwGetMouseButton( m_window, GLFW_MOUSE_BUTTON_LEFT ) == GLFW_PRESS;
    if( m_controller != nullptr && m_controller->isButtonPressed( ControllerButton::x ) )
        m_inputState.action = true;

    m_inputState.holster = glfwGetKey( m_window, GLFW_KEY_R ) == GLFW_PRESS
                           || glfwGetMouseButton( m_window, GLFW_MOUSE_BUTTON_MIDDLE ) == GLFW_PRESS
                           || (m_controller != nullptr
                               && m_controller->isButtonPressed( ControllerButton::rightstick ));

    m_inputState.jump = glfwGetKey( m_window, GLFW_KEY_SPACE ) == GLFW_PRESS;
    if( m_controller != nullptr && m_controller->isButtonPressed( ControllerButton::a ) )
        m_inputState.jump = true;

    m_inputState.roll = glfwGetKey( m_window, GLFW_KEY_X ) == GLFW_PRESS;
    if( m_controller != nullptr && m_controller->isButtonPressed( ControllerButton::back ) )
        m_inputState.roll = true;

    m_inputState.freeLook = glfwGetMouseButton( m_window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS;
    if( m_controller != nullptr && m_controller->isButtonPressed( ControllerButton::y ) )
        m_inputState.freeLook = true;

    m_inputState.debug = glfwGetKey( m_window, GLFW_KEY_F11 ) == GLFW_PRESS;

    m_inputState._1 = glfwGetKey( m_window, GLFW_KEY_1 ) == GLFW_PRESS
                      || (m_controller != nullptr
                          && m_controller->getAxisValue( ControllerAxis::righty ) < -AxisThreshold);
    m_inputState._2 = glfwGetKey( m_window, GLFW_KEY_2 ) == GLFW_PRESS
                      || (m_controller != nullptr
                          && m_controller->getAxisValue( ControllerAxis::righty ) > AxisThreshold);
    m_inputState._3 = glfwGetKey( m_window, GLFW_KEY_3 ) == GLFW_PRESS
                      || (m_controller != nullptr
                          && m_controller->getAxisValue( ControllerAxis::rightx ) < -AxisThreshold);
    m_inputState._4 = glfwGetKey( m_window, GLFW_KEY_4 ) == GLFW_PRESS
                      || (m_controller != nullptr
                          && m_controller->getAxisValue( ControllerAxis::rightx ) > AxisThreshold);
    m_inputState._5 = glfwGetKey( m_window, GLFW_KEY_5 ) == GLFW_PRESS;
    m_inputState._6 = glfwGetKey( m_window, GLFW_KEY_6 ) == GLFW_PRESS;

    m_inputState.save = glfwGetKey( m_window, GLFW_KEY_F5 ) == GLFW_PRESS;
    m_inputState.load = glfwGetKey( m_window, GLFW_KEY_F6 ) == GLFW_PRESS;

    double x, y;
    glfwGetCursorPos( m_window, &x, &y );

    if( m_controller != nullptr )
    {
        if( m_inputState.freeLook )
        {
            x += m_controller->getAxisValue( ControllerAxis::leftx ) * 500;
            y -= m_controller->getAxisValue( ControllerAxis::lefty ) * 500;
        }
        else
        {
            if( m_controller->getAxisValue( ControllerAxis::lefty ) < -AxisThreshold )
                forward = true;
            if( m_controller->getAxisValue( ControllerAxis::lefty ) > AxisThreshold )
                backward = true;
            if( m_controller->getAxisValue( ControllerAxis::leftx ) < -AxisThreshold )
                left = true;
            if( m_controller->getAxisValue( ControllerAxis::leftx ) > AxisThreshold )
                right = true;
        }
    }

    m_inputState.mouseMovement = glm::vec2( x - std::exchange( m_lastCursorX, x ),
                                            y - std::exchange( m_lastCursorY, y ) );

    m_inputState.setXAxisMovement( left, right );
    m_inputState.setZAxisMovement( backward, forward );
    m_inputState.setStepMovement( stepLeft, stepRight );
}
}
