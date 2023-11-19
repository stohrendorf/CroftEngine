#pragma once

#include "audiosettings.h"
#include "displaysettings.h"
#include "hid/actions.h"
#include "hid/glfw_axes.h"
#include "hid/glfw_axis_dirs.h"
#include "hid/glfw_gamepad_buttons.h"
#include "hid/glfw_keys.h"
#include "render/rendersettings.h"
#include "serialization/named_enum.h"
#include "serialization/serialization_fwd.h"

#include <algorithm>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace engine
{
using NamedGlfwKey = serialization::NamedEnum<hid::GlfwKey, hid::EnumUtil<hid::GlfwKey>>;
using NamedGlfwGamepadButton = serialization::NamedEnum<hid::GlfwGamepadButton, hid::EnumUtil<hid::GlfwGamepadButton>>;
using NamedGlfwAxis = serialization::NamedEnum<hid::GlfwAxis, hid::EnumUtil<hid::GlfwAxis>>;
using NamedGlfwAxisDir = serialization::NamedEnum<hid::GlfwAxisDir, hid::EnumUtil<hid::GlfwAxisDir>>;
using NamedAction = serialization::NamedEnum<hid::Action, hid::EnumUtil<hid::Action>>;
using NamedAxisDir = std::pair<NamedGlfwAxis, NamedGlfwAxisDir>;

using InputMappingConfig = std::map<std::variant<NamedGlfwKey, NamedGlfwGamepadButton, NamedAxisDir>, NamedAction>;

struct EngineConfig;

struct NamedInputMappingConfig
{
  std::string name;
  std::string controllerType;
  InputMappingConfig gameMappings;
  InputMappingConfig menuMappings;

  void serialize(const serialization::Serializer<EngineConfig>& ser) const;
  void deserialize(const serialization::Deserializer<EngineConfig>& ser);
  static NamedInputMappingConfig create(const serialization::Deserializer<EngineConfig>& ser);

  bool operator==(const NamedInputMappingConfig& rhs) const
  {
    return name == rhs.name && controllerType == rhs.controllerType && gameMappings == rhs.gameMappings
           && menuMappings == rhs.menuMappings;
  }
};

struct EngineConfig
{
  render::RenderSettings renderSettings{};
  DisplaySettings displaySettings{};
  AudioSettings audioSettings{};
  std::vector<NamedInputMappingConfig> inputMappings{};
  bool restoreHealth = true;
  bool pulseLowHealthHealthBar = true;
  bool lowHealthMonochrome = true;
  bool buttBubbles = false;
  bool waterBedBubbles = true;
  bool animSmoothing = true;
  bool saveReminderEnabled = false;
  uint8_t saveReminderMinutes = 15;
  bool selectFirstFreeOrOldestSlot = false;
  bool delaySaveEnabled = false;
  uint8_t delaySaveDurationSeconds = 3;

  explicit EngineConfig();

  void serialize(const serialization::Serializer<EngineConfig>& ser) const;
  void deserialize(const serialization::Deserializer<EngineConfig>& ser);

  void resetInputMappings(bool modern);
};
} // namespace engine
