#include "engineconfig.h"

#include "serialization/default.h"
#include "serialization/map.h"
#include "serialization/serialization.h"
#include "serialization/variant.h"

namespace engine
{
namespace
{
using hid::Action;
using hid::EnumUtil;
using hid::GlfwGamepadButton;
using hid::GlfwKey;

const InputMappingConfig defaultMapping{
  {GlfwKey::LeftShift, Action::Walk},
  {GlfwGamepadButton::RightBumper, Action::Walk},
  {GlfwKey::LeftControl, Action::Action},
  {GlfwGamepadButton::A, Action::Action},
  {GlfwKey::R, Action::Holster},
  {GlfwGamepadButton::Y, Action::Holster},
  {GlfwKey::Space, Action::Jump},
  {GlfwGamepadButton::X, Action::Jump},
  {GlfwKey::X, Action::Roll},
  {GlfwGamepadButton::B, Action::Roll},
  {GlfwKey::Kp0, Action::FreeLook},
  {GlfwGamepadButton::LeftBumper, Action::FreeLook},
  {GlfwKey::Escape, Action::Menu},
  {GlfwGamepadButton::Start, Action::Menu},
  {GlfwKey::F11, Action::Debug},
  {GlfwKey::Num1, Action::DrawPistols},
  {GlfwKey::Num2, Action::DrawShotgun},
  {GlfwKey::Num3, Action::DrawUzis},
  {GlfwKey::Num4, Action::DrawMagnums},
  {GlfwKey::Num5, Action::ConsumeSmallMedipack},
  {GlfwKey::Num6, Action::ConsumeLargeMedipack},
  {GlfwKey::F5, Action::Save},
  {GlfwKey::F6, Action::Load},
  {GlfwKey::A, Action::Left},
  {GlfwGamepadButton::DPadLeft, Action::Left},
  {GlfwKey::D, Action::Right},
  {GlfwGamepadButton::DPadRight, Action::Right},
  {GlfwKey::W, Action::Forward},
  {GlfwGamepadButton::DPadUp, Action::Forward},
  {GlfwKey::S, Action::Backward},
  {GlfwGamepadButton::DPadDown, Action::Backward},
  {GlfwKey::Q, Action::StepLeft},
  {GlfwKey::E, Action::StepRight},
  {GlfwKey::F12, Action::Screenshot},
  {GlfwKey::F10, Action::CheatDive}, // only available in debug builds
};
} // namespace

void EngineConfig::serialize(const serialization::Serializer<EngineConfig>& ser)
{
  ser(S_NVD("renderSettings", renderSettings, render::RenderSettings{}),
      S_NVD("displaySettings", displaySettings, DisplaySettings{}),
      S_NVD("inputMapping", inputMapping, defaultMapping));
}

EngineConfig::EngineConfig()
    : inputMapping{defaultMapping}
{
}
} // namespace engine
