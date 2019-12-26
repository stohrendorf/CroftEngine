#pragma once

#include "shaderprogram.h"

#include <boost/algorithm/string/join.hpp>
#include <filesystem>
#include <unordered_map>
#include <utility>

namespace render::scene
{
class ShaderManager final
{
  std::unordered_map<std::string, gsl::not_null<std::shared_ptr<ShaderProgram>>> m_programs{};

  const std::filesystem::path m_root;

public:
  explicit ShaderManager(std::filesystem::path root)
      : m_root{std::move(root)}
  {
  }

  static std::string makeId(const std::filesystem::path& vshPath,
                            const std::filesystem::path& fshPath,
                            const std::vector<std::string>& defines);

  gsl::not_null<std::shared_ptr<ShaderProgram>> get(const std::filesystem::path& vshPath,
                                                    const std::filesystem::path& fshPath,
                                                    const std::vector<std::string>& defines = {});

  auto getScreenOverlay()
  {
    return get("screenoverlay.vert", "screenoverlay.frag", {});
  }

  auto getTextured()
  {
    return get("textured_2.vert", "textured_2.frag");
  }

  auto getTexturedWater()
  {
    return get("textured_2.vert", "textured_2.frag", {"WATER"});
  }

  auto getColored()
  {
    return get("colored_2.vert", "colored_2.frag");
  }

  auto getDepthOnly()
  {
    return get("depth_only.vert", "empty.frag");
  }

  auto getPortal()
  {
    return get("portal.vert", "portal.frag");
  }

  auto getFXAA()
  {
    return get("flat.vert", "fxaa.frag");
  }

  auto getSSAO()
  {
    return get("flat.vert", "ssao.frag");
  }

  auto getSSAOBlur()
  {
    return get("flat.vert", "ssao_blur.frag");
  }

  auto getPostprocessing()
  {
    return get("flat.vert", "fx_darkness.frag", {"LENS_DISTORTION"});
  }

  auto getPostprocessingWater()
  {
    return get("flat.vert", "fx_darkness.frag", {"WATER", "LENS_DISTORTION"});
  }

  auto getLightning()
  {
    return get("lightning.vert", "lightning.frag");
  }
};
} // namespace render::scene
