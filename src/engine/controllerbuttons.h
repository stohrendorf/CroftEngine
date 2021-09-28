#pragma once

#include "hid/glfw_axes.h"
#include "hid/glfw_gamepad_buttons.h"
#include "world/sprite.h"

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <variant>

namespace render
{
class MultiTextureAtlas;
}

namespace render::scene
{
class Material;
}

namespace engine
{
using ControllerLayout = std::map<std::variant<hid::GlfwGamepadButton, hid::GlfwAxis>, world::Sprite>;
using ControllerLayouts = std::map<std::string, ControllerLayout>;

extern ControllerLayouts
  loadControllerButtonIcons(render::MultiTextureAtlas& atlases,
                            const std::filesystem::path& configFile,
                            const gsl::not_null<std::shared_ptr<render::scene::Material>>& material);
} // namespace engine
