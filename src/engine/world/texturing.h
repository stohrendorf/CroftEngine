#pragma once

#include <filesystem>
#include <functional>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <memory>
#include <string>
#include <vector>

namespace loader::file::level
{
class Level;
}

namespace loader::trx
{
class Glidos;
}

namespace render
{
class MultiTextureAtlas;
}

namespace engine::world
{
struct AtlasTile;
struct Sprite;

extern std::unique_ptr<gl::Texture2DArray<gl::PremultipliedSRGBA8>>
  buildTextures(const loader::file::level::Level& level,
                const std::unique_ptr<loader::trx::Glidos>& glidos,
                render::MultiTextureAtlas& atlases,
                std::vector<AtlasTile>& atlasTiles,
                std::vector<Sprite>& sprites,
                const std::function<void(const std::string&)>& drawLoadingScreen,
                const std::filesystem::path& cacheDir);
} // namespace engine::world
