#pragma once

#include <irrlicht.h>

#include "inputstate.h"

namespace engine
{
class InputHandler final : public irr::IEventReceiver
{
public:
    explicit InputHandler(irr::gui::ICursorControl* cursorControl)
        : m_cursorControl(cursorControl)
    {
        cursorControl->setPosition(0.5f, 0.5f);
    }

    const InputState& getInputState()
    {
        m_inputState.setXAxisMovement(m_left, m_right);
        m_inputState.setZAxisMovement(m_backward, m_forward);
        m_inputState.setStepMovement(m_stepLeft, m_stepRight);

        m_inputState.mouseMovement = m_cursorControl->getRelativePosition() - irr::core::vector2df{ 0.5f, 0.5f };
        m_cursorControl->setPosition(0.5f, 0.5f);

        return m_inputState;
    }

    bool OnEvent(const irr::SEvent& evt) override
    {
        switch(evt.EventType)
        {
            case irr::EET_KEY_INPUT_EVENT:
                switch(evt.KeyInput.Key)
                {
                    case irr::KEY_KEY_A:
                        m_left = evt.KeyInput.PressedDown;
                        return true;
                    case irr::KEY_KEY_D:
                        m_right = evt.KeyInput.PressedDown;
                        return true;
                    case irr::KEY_KEY_Q:
                        m_stepLeft = evt.KeyInput.PressedDown;
                        return true;
                    case irr::KEY_KEY_E:
                        m_stepRight = evt.KeyInput.PressedDown;
                        return true;
                    case irr::KEY_KEY_W:
                        m_forward = evt.KeyInput.PressedDown;
                        return true;
                    case irr::KEY_KEY_S:
                        m_backward = evt.KeyInput.PressedDown;
                        return true;
                    case irr::KEY_LSHIFT:
                    case irr::KEY_RSHIFT:
                    case irr::KEY_SHIFT:
                        m_inputState.moveSlow = evt.KeyInput.PressedDown;
                        return true;
                    case irr::KEY_LCONTROL:
                    case irr::KEY_RCONTROL:
                    case irr::KEY_CONTROL:
                        m_inputState.action = evt.KeyInput.PressedDown;
                        return true;
                    case irr::KEY_SPACE:
                        m_inputState.jump = evt.KeyInput.PressedDown;
                        return true;
                    case irr::KEY_KEY_X:
                        m_inputState.roll = evt.KeyInput.PressedDown;
                        break;
                    default:
                        return false;
                }
                break;

            case irr::EET_MOUSE_INPUT_EVENT:
                switch(evt.MouseInput.Event)
                {
                    case irr::EMIE_LMOUSE_PRESSED_DOWN:
                        m_inputState.action = true;
                        return true;
                    case irr::EMIE_LMOUSE_LEFT_UP:
                        m_inputState.action = false;
                        return true;
                    default:
                        return false;
                }

            default:
                break;
        }

        return false;
    }

private:
    irr::gui::ICursorControl* const m_cursorControl;

    bool m_forward = false;
    bool m_backward = false;
    bool m_left = false;
    bool m_right = false;
    bool m_stepLeft = false;
    bool m_stepRight = false;

    InputState m_inputState;
};
}
