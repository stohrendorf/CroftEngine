#pragma once

#include "hid/glfw_gamepad_buttons.h"
#include "world/sprite.h"

#include <filesystem>
#include <map>
#include <memory>
#include <string>

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
using ControllerLayout = std::map<hid::GlfwGamepadButton, world::Sprite>;
using ControllerLayouts = std::map<std::string, ControllerLayout>;

extern ControllerLayouts loadControllerButtonIcons(render::MultiTextureAtlas& atlases,
                                                   const std::filesystem::path& configFile,
                                                   const std::shared_ptr<render::scene::Material>& material);
} // namespace engine
