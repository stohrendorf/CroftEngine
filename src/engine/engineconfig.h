#pragma once

#include "displaysettings.h"
#include "hid/actions.h"
#include "hid/glfw_axes.h"
#include "hid/glfw_axis_dirs.h"
#include "hid/glfw_gamepad_buttons.h"
#include "hid/glfw_keys.h"
#include "render/rendersettings.h"
#include "serialization/named_enum.h"

#include <map>
#include <variant>

namespace engine
{
using NamedGlfwKey = serialization::NamedEnum<hid::GlfwKey, hid::EnumUtil<hid::GlfwKey>>;
using NamedGlfwGamepadButton = serialization::NamedEnum<hid::GlfwGamepadButton, hid::EnumUtil<hid::GlfwGamepadButton>>;
using NamedGlfwAxis = serialization::NamedEnum<hid::GlfwAxis, hid::EnumUtil<hid::GlfwAxis>>;
using NamedGlfwAxisDir = serialization::NamedEnum<hid::GlfwAxisDir, hid::EnumUtil<hid::GlfwAxisDir>>;
using NamedAction = serialization::NamedEnum<hid::Action, hid::EnumUtil<hid::Action>>;
using NamedAxisDir = std::pair<NamedGlfwAxis, NamedGlfwAxisDir>;

using InputMappingConfig = std::map<std::variant<NamedGlfwKey, NamedGlfwGamepadButton, NamedAxisDir>, NamedAction>;

struct NamedInputMappingConfig
{
  std::string name;
  std::string controllerType;
  InputMappingConfig mappings;

  void serialize(const serialization::Serializer<EngineConfig>& ser);
  static NamedInputMappingConfig create(const serialization::Serializer<EngineConfig>& ser);

  bool operator==(const NamedInputMappingConfig& rhs) const
  {
    return name == rhs.name && controllerType == rhs.controllerType && mappings == rhs.mappings;
  }
};

struct EngineConfig
{
  render::RenderSettings renderSettings{};
  DisplaySettings displaySettings{};
  std::vector<NamedInputMappingConfig> inputMappings{};

  explicit EngineConfig();

  void serialize(const serialization::Serializer<EngineConfig>& ser);

  void resetInputMappings();
};
} // namespace engine
