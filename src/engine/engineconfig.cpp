#include "engineconfig.h"

#include "core/i18n.h"
#include "engine/audiosettings.h"
#include "engine/displaysettings.h"
#include "serialization/default.h"
#include "serialization/map.h"
#include "serialization/optional_value.h"
#include "serialization/pair.h"
#include "serialization/serialization.h"
#include "serialization/variant.h"
#include "serialization/vector.h"

#include <exception>
#include <stdexcept>

namespace engine
{
namespace
{
using hid::Action;
using hid::GlfwAxis;
using hid::GlfwAxisDir;
using hid::GlfwGamepadButton;
using hid::GlfwKey;

std::vector<NamedInputMappingConfig> getDefaultMappings()
{
  return {
    {
      pgettext("Input|MappingName", "Keyboard"),
      "PS",
      {
        {GlfwKey::LeftShift, Action::Walk},
        {GlfwKey::LeftControl, Action::Action},
        {GlfwKey::R, Action::Holster},
        {GlfwKey::Space, Action::Jump},
        {GlfwKey::X, Action::Roll},
        {GlfwKey::Kp0, Action::FreeLook},
        {GlfwKey::Escape, Action::Menu},
        {GlfwKey::Num1, Action::DrawPistols},
        {GlfwKey::Num2, Action::DrawShotgun},
        {GlfwKey::Num3, Action::DrawUzis},
        {GlfwKey::Num4, Action::DrawMagnums},
        {GlfwKey::Num5, Action::ConsumeSmallMedipack},
        {GlfwKey::Num6, Action::ConsumeLargeMedipack},
        {GlfwKey::F5, Action::Save},
        {GlfwKey::F6, Action::Load},
        {GlfwKey::A, Action::Left},
        {GlfwKey::D, Action::Right},
        {GlfwKey::W, Action::Forward},
        {GlfwKey::S, Action::Backward},
        {GlfwKey::Q, Action::StepLeft},
        {GlfwKey::E, Action::StepRight},
        {GlfwKey::F12, Action::Screenshot},
        {GlfwKey::F1, Action::BugReport},
      },
      {
        {GlfwKey::A, Action::MenuLeft},
        {GlfwKey::D, Action::MenuRight},
        {GlfwKey::W, Action::MenuUp},
        {GlfwKey::S, Action::MenuDown},
        {GlfwKey::Q, Action::PrevScreen},
        {GlfwKey::E, Action::NextScreen},
        {GlfwKey::LeftControl, Action::PrimaryInteraction},
        {GlfwKey::R, Action::SecondaryInteraction},
        {GlfwKey::Escape, Action::Return},
        {GlfwKey::F10, Action::CheatDive} // only available in debug builds
      },
    },
    {
      pgettext("Input|MappingName", "Gamepad"),
      "PS",
      {
        {GlfwGamepadButton::RightBumper, Action::Walk},
        {GlfwGamepadButton::A, Action::Action},
        {GlfwGamepadButton::Y, Action::Holster},
        {GlfwGamepadButton::X, Action::Jump},
        {GlfwGamepadButton::B, Action::Roll},
        {GlfwGamepadButton::LeftBumper, Action::FreeLook},
        {GlfwGamepadButton::Start, Action::Menu},
        {GlfwGamepadButton::DPadLeft, Action::Left},
        {GlfwGamepadButton::DPadRight, Action::Right},
        {GlfwGamepadButton::DPadUp, Action::Forward},
        {GlfwGamepadButton::DPadDown, Action::Backward},
        {NamedAxisDir{GlfwAxis::LeftTrigger, GlfwAxisDir::Positive}, Action::StepLeft},
        {NamedAxisDir{GlfwAxis::RightTrigger, GlfwAxisDir::Positive}, Action::StepRight},
      },
      {
        {GlfwGamepadButton::DPadLeft, Action::MenuLeft},
        {GlfwGamepadButton::DPadRight, Action::MenuRight},
        {GlfwGamepadButton::DPadUp, Action::MenuUp},
        {GlfwGamepadButton::DPadDown, Action::MenuDown},
        {GlfwGamepadButton::LeftBumper, Action::PrevScreen},
        {GlfwGamepadButton::RightBumper, Action::NextScreen},
        {GlfwGamepadButton::A, Action::PrimaryInteraction},
        {GlfwGamepadButton::Y, Action::SecondaryInteraction},
        {GlfwGamepadButton::B, Action::Return},
      },
    },
  };
}
} // namespace

void NamedInputMappingConfig::serialize(const serialization::Serializer<EngineConfig>& ser) const
{
  ser(S_NV("name", name),
      S_NV("controllerType", controllerType),
      S_NV("gameMappings", gameMappings),
      S_NV("menuMappings", menuMappings));
}

void NamedInputMappingConfig::deserialize(const serialization::Deserializer<EngineConfig>& ser)
{
  // TODO CE-601 remove migration
  ser(S_NV("name", name),
      S_NV("controllerType", controllerType),
      S_NVO("mappings", std::ref(gameMappings)),
      S_NVO("gameMappings", std::ref(gameMappings)));

  ser(S_NVD("menuMappings",
            std::ref(menuMappings),
            getDefaultMappings().at(name == pgettext("Input|MappingName", "Keyboard") ? 0 : 1).menuMappings));
}

NamedInputMappingConfig NamedInputMappingConfig::create(const serialization::Deserializer<EngineConfig>& ser)
{
  NamedInputMappingConfig tmp{};
  tmp.deserialize(ser);
  return tmp;
}

void EngineConfig::serialize(const serialization::Serializer<EngineConfig>& ser) const
{
  ser(S_NV("renderSettings", renderSettings),
      S_NV("displaySettings", displaySettings),
      S_NV("audioSettings", audioSettings),
      S_NV("inputMappings", inputMappings),
      S_NV("restoreHealth", restoreHealth),
      S_NV("pulseLowHealthHealthBar", pulseLowHealthHealthBar),
      S_NV("lowHealthMonochrome", lowHealthMonochrome),
      S_NV("buttBubbles", buttBubbles),
      S_NV("waterBedBubbles", waterBedBubbles),
      S_NV("animSmoothing", animSmoothing));
}

void EngineConfig::deserialize(const serialization::Deserializer<EngineConfig>& ser)
{
  ser(S_NVD("renderSettings", std::ref(renderSettings), render::RenderSettings{}),
      S_NVD("displaySettings", std::ref(displaySettings), DisplaySettings{}),
      S_NVD("audioSettings", std::ref(audioSettings), AudioSettings{}),
      S_NVD("inputMappings", std::ref(inputMappings), getDefaultMappings()),
      S_NVO("restoreHealth", std::ref(restoreHealth)),
      S_NVO("pulseLowHealthHealthBar", std::ref(pulseLowHealthHealthBar)),
      S_NVO("lowHealthMonochrome", std::ref(lowHealthMonochrome)),
      S_NVO("buttBubbles", std::ref(buttBubbles)),
      S_NVO("waterBedBubbles", std::ref(waterBedBubbles)),
      S_NVO("animSmoothing", std::ref(animSmoothing)));
}

EngineConfig::EngineConfig()
    : inputMappings{getDefaultMappings()}
{
}

void EngineConfig::resetInputMappings()
{
  inputMappings = getDefaultMappings();
}
} // namespace engine
