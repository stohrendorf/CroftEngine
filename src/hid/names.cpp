#include "names.h"

#include "actions.h"
#include "core/i18n.h"
#include "glfw_gamepad_buttons.h"
#include "glfw_keys.h"
#include "ui/util.h"

#include <boost/throw_exception.hpp>
#include <stdexcept>

namespace hid
{
std::string getName(GlfwKey key)
{
  switch(key)
  {
  case GlfwKey::Space: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Space");
  case GlfwKey::Apostrophe: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "'");
  case GlfwKey::Comma: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", ",");
  case GlfwKey::Minus: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "-");
  case GlfwKey::Period: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", ".");
  case GlfwKey::Slash: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "/");
  case GlfwKey::Num0: return "0";
  case GlfwKey::Num1: return "1";
  case GlfwKey::Num2: return "2";
  case GlfwKey::Num3: return "3";
  case GlfwKey::Num4: return "4";
  case GlfwKey::Num5: return "5";
  case GlfwKey::Num6: return "6";
  case GlfwKey::Num7: return "7";
  case GlfwKey::Num8: return "8";
  case GlfwKey::Num9: return "9";
  case GlfwKey::Semicolon: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", ";");
  case GlfwKey::Equal: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Equal");
  case GlfwKey::A: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "A");
  case GlfwKey::B: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "B");
  case GlfwKey::C: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "C");
  case GlfwKey::D: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "D");
  case GlfwKey::E: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "E");
  case GlfwKey::F: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "F");
  case GlfwKey::G: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "G");
  case GlfwKey::H: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "H");
  case GlfwKey::I: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "I");
  case GlfwKey::J: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "J");
  case GlfwKey::K: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "K");
  case GlfwKey::L: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "L");
  case GlfwKey::M: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "M");
  case GlfwKey::N: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "N");
  case GlfwKey::O: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "O");
  case GlfwKey::P: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "P");
  case GlfwKey::Q: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Q");
  case GlfwKey::R: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "R");
  case GlfwKey::S: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "S");
  case GlfwKey::T: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "T");
  case GlfwKey::U: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "U");
  case GlfwKey::V: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "V");
  case GlfwKey::W: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "W");
  case GlfwKey::X: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "X");
  case GlfwKey::Y: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Y");
  case GlfwKey::Z: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Z");
  case GlfwKey::LeftBracket:
    return /* translators: TR charmap encoding (square bracket left) */ pgettext("Keyboard|Key", "<");
  case GlfwKey::Backslash: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "\\");
  case GlfwKey::RightBracket:
    return /* translators: TR charmap encoding (square bracket right) */ pgettext("Keyboard|Key", ">");
  case GlfwKey::GraveAccent:
    return /* translators: TR charmap encoding (grave accent) */ pgettext("Keyboard|Key", "( ");
  case GlfwKey::Escape: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Esc");
  case GlfwKey::Enter: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Ret");
  case GlfwKey::Tab: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Tab");
  case GlfwKey::Backspace: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Bksp");
  case GlfwKey::Insert: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Ins");
  case GlfwKey::Delete: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Del");
  case GlfwKey::Right: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Right");
  case GlfwKey::Left: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Left");
  case GlfwKey::Down: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Down");
  case GlfwKey::Up: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Up");
  case GlfwKey::PageUp: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "PgUp");
  case GlfwKey::PageDown: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "PgDn");
  case GlfwKey::Home: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Home");
  case GlfwKey::End: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "End");
  case GlfwKey::CapsLock: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "CapsLk");
  case GlfwKey::ScrollLock: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "ScrlLk");
  case GlfwKey::NumLock: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "NmLk");
  case GlfwKey::PrintScreen: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "PrtScrn");
  case GlfwKey::Pause: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Pause");
  case GlfwKey::F1: return "F1";
  case GlfwKey::F2: return "F2";
  case GlfwKey::F3: return "F3";
  case GlfwKey::F4: return "F4";
  case GlfwKey::F5: return "F5";
  case GlfwKey::F6: return "F6";
  case GlfwKey::F7: return "F7";
  case GlfwKey::F8: return "F8";
  case GlfwKey::F9: return "F9";
  case GlfwKey::F10: return "F10";
  case GlfwKey::F11: return "F11";
  case GlfwKey::F12: return "F12";
  case GlfwKey::F13: return "F13";
  case GlfwKey::F14: return "F14";
  case GlfwKey::F15: return "F15";
  case GlfwKey::F16: return "F16";
  case GlfwKey::F17: return "F17";
  case GlfwKey::F18: return "F18";
  case GlfwKey::F19: return "F19";
  case GlfwKey::F20: return "F20";
  case GlfwKey::F21: return "F21";
  case GlfwKey::F22: return "F22";
  case GlfwKey::F23: return "F23";
  case GlfwKey::F24: return "F24";
  case GlfwKey::F25: return "F25";
  case GlfwKey::Kp0: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Pad0");
  case GlfwKey::Kp1: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Pad1");
  case GlfwKey::Kp2: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Pad2");
  case GlfwKey::Kp3: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Pad3");
  case GlfwKey::Kp4: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Pad4");
  case GlfwKey::Kp5: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Pad5");
  case GlfwKey::Kp6: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Pad6");
  case GlfwKey::Kp7: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Pad7");
  case GlfwKey::Kp8: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Pad8");
  case GlfwKey::Kp9: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Pad9");
  case GlfwKey::KpDecimal: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Pad.");
  case GlfwKey::KpDivide: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Pad/");
  case GlfwKey::KpMultiply: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Padx");
  case GlfwKey::KpSubtract: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Pad-");
  case GlfwKey::KpAdd: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Pad+");
  case GlfwKey::KpEnter: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "PadEnter");
  case GlfwKey::LeftShift: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Shift");
  case GlfwKey::LeftControl: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Ctrl");
  case GlfwKey::LeftAlt: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Alt");
  case GlfwKey::RightShift: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "RShift");
  case GlfwKey::RightControl: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "RCtrl");
  case GlfwKey::RightAlt: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "RAlt");
  case GlfwKey::World1: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "World1");
  case GlfwKey::World2: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "World2");
  case GlfwKey::KpEqual: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "PadEq");
  case GlfwKey::LeftSuper: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Super");
  case GlfwKey::RightSuper: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "RSuper");
  case GlfwKey::Menu: return /* translators: TR charmap encoding */ pgettext("Keyboard|Key", "Menu");
  }
  BOOST_THROW_EXCEPTION(std::domain_error("key"));
}

std::string getName(GlfwGamepadButton button)
{
  switch(button)
  {
  case GlfwGamepadButton::A: return ui::getSpriteSelector(ui::GamepadCrossSprite);
  case GlfwGamepadButton::B: return ui::getSpriteSelector(ui::GamepadCircleSprite);
  case GlfwGamepadButton::X: return ui::getSpriteSelector(ui::GamepadSquareSprite);
  case GlfwGamepadButton::Y: return ui::getSpriteSelector(ui::GamepadTriangleSprite);
  case GlfwGamepadButton::LeftBumper: return ui::getSpriteSelector(ui::GamepadL1Sprite);
  case GlfwGamepadButton::RightBumper: return ui::getSpriteSelector(ui::GamepadR1Sprite);
  case GlfwGamepadButton::Back:
    return ui::getSpriteSelector(ui::GamepadEmptyButtonSprite)
           + /* translators: TR charmap encoding, gamepad */ pgettext("Gamepad|Button", "Back");
  case GlfwGamepadButton::Start:
    return ui::getSpriteSelector(ui::GamepadEmptyButtonSprite)
           + /* translators: TR charmap encoding, gamepad */ pgettext("Gamepad|Button", "Start");
  case GlfwGamepadButton::Guide:
    return ui::getSpriteSelector(ui::GamepadEmptyButtonSprite)
           + /* translators: TR charmap encoding, gamepad */ pgettext("Gamepad|Button", "Guide");
  case GlfwGamepadButton::DPadUp: return ui::getSpriteSelector(ui::GamepadDPadUpSprite);
  case GlfwGamepadButton::DPadRight: return ui::getSpriteSelector(ui::GamepadDPadRightSprite);
  case GlfwGamepadButton::DPadDown: return ui::getSpriteSelector(ui::GamepadDPadDownSprite);
  case GlfwGamepadButton::DPadLeft: return ui::getSpriteSelector(ui::GamepadDPadLeftSprite);
  case GlfwGamepadButton::LeftThumb:
    return ui::getSpriteSelector(ui::GamepadEmptyButtonSprite)
           + /* translators: TR charmap encoding, gamepad */ pgettext("Gamepad|Button", "LThumb");
  case GlfwGamepadButton::RightThumb:
    return ui::getSpriteSelector(ui::GamepadEmptyButtonSprite)
           + /* translators: TR charmap encoding, gamepad */ pgettext("Gamepad|Button", "RThumb");
  }
  BOOST_THROW_EXCEPTION(std::domain_error("button"));
}

std::string getName(Action action)
{
  switch(action)
  {
  case Action::Jump: return /* translators: TR charmap encoding */ pgettext("Action", "Jump");
  case Action::Walk: return /* translators: TR charmap encoding */ pgettext("Action", "Walk");
  case Action::Roll: return /* translators: TR charmap encoding */ pgettext("Action", "Roll");
  case Action::Action: return /* translators: TR charmap encoding */ pgettext("Action", "Action");
  case Action::FreeLook: return /* translators: TR charmap encoding */ pgettext("Action", "Look");
  case Action::Debug: return /* translators: TR charmap encoding */ pgettext("Action", "Debug");
  case Action::Holster: return /* translators: TR charmap encoding */ pgettext("Action", "Draw Weapon");
  case Action::Menu: return /* translators: TR charmap encoding */ pgettext("Action", "Inventory");
  case Action::DrawPistols: return /* translators: TR charmap encoding */ pgettext("Action", "Draw Pistols");
  case Action::DrawShotgun: return /* translators: TR charmap encoding */ pgettext("Action", "Draw Shotgun");
  case Action::DrawUzis: return /* translators: TR charmap encoding */ pgettext("Action", "Draw Uzis");
  case Action::DrawMagnums: return /* translators: TR charmap encoding */ pgettext("Action", "Draw Magnums");
  case Action::ConsumeSmallMedipack: return /* translators: TR charmap encoding */ pgettext("Action", "Small Medipack");
  case Action::ConsumeLargeMedipack: return /* translators: TR charmap encoding */ pgettext("Action", "Large Medipack");
  case Action::Save: return /* translators: TR charmap encoding */ pgettext("Action", "Save");
  case Action::Load: return /* translators: TR charmap encoding */ pgettext("Action", "Load");
  case Action::Left: return /* translators: TR charmap encoding */ pgettext("Action", "Left");
  case Action::Right: return /* translators: TR charmap encoding */ pgettext("Action", "Right");
  case Action::Forward: return /* translators: TR charmap encoding */ pgettext("Action", "Run");
  case Action::Backward: return /* translators: TR charmap encoding */ pgettext("Action", "Back");
  case Action::StepLeft: return /* translators: TR charmap encoding */ pgettext("Action", "Step Left");
  case Action::StepRight: return /* translators: TR charmap encoding */ pgettext("Action", "Step Right");
  case Action::CheatDive: return /* translators: TR charmap encoding */ pgettext("Action", "Cheat Dive");
  case Action::Screenshot: return /* translators: TR charmap encoding */ pgettext("Action", "Screenshot");
  }
  BOOST_THROW_EXCEPTION(std::domain_error("action"));
}
} // namespace hid
