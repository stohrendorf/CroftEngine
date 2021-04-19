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
struct EngineConfig
{
  render::RenderSettings renderSettings{};
  DisplaySettings displaySettings{};
  std::map<std::variant<serialization::NamedEnum<hid::GlfwKey, hid::EnumUtil<hid::GlfwKey>>,
                        serialization::NamedEnum<hid::GlfwGamepadButton, hid::EnumUtil<hid::GlfwGamepadButton>>>,
           serialization::NamedEnum<hid::Action, hid::EnumUtil<hid::Action>>>
    inputMapping;

  explicit EngineConfig();

  void serialize(const serialization::Serializer<EngineConfig>& ser);
};
} // namespace engine
