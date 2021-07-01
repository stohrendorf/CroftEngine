#pragma once

#include "displaysettings.h"
#include "hid/actions.h"
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
using NamedAction = serialization::NamedEnum<hid::Action, hid::EnumUtil<hid::Action>>;

using InputMappingConfig = std::map<std::variant<NamedGlfwKey, NamedGlfwGamepadButton>, NamedAction>;

struct NamedInputMappingConfig
{
  std::string name;
  InputMappingConfig mappings;

  void serialize(const serialization::Serializer<EngineConfig>& ser);
  static NamedInputMappingConfig create(const serialization::Serializer<EngineConfig>& ser);
};

struct EngineConfig
{
  render::RenderSettings renderSettings{};
  DisplaySettings displaySettings{};
  std::vector<NamedInputMappingConfig> inputMappings{};
  size_t activeInputMapping = 0;

  explicit EngineConfig();

  void serialize(const serialization::Serializer<EngineConfig>& ser);
};
} // namespace engine
