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

  auto getFlat()
  {
    return get("flat.vert", "flat.frag", {"INVERT_Y"});
  }

  auto getGeometry(bool water, bool skeletal)
  {
    std::vector<std::string> defines;
    if(water)
      defines.emplace_back("WATER");
    if(skeletal)
      defines.emplace_back("SKELETAL");
    return get("geometry.vert", "geometry.frag", defines);
  }

  auto getCSMDepthOnly(bool skeletal)
  {
    std::vector<std::string> defines;
    if(skeletal)
      defines.emplace_back("SKELETAL");
    return get("csm_depth_only.vert", "empty.frag", defines);
  }

  auto getDepthOnly(bool skeletal)
  {
    std::vector<std::string> defines;
    if(skeletal)
      defines.emplace_back("SKELETAL");
    return get("depth_only.vert", "empty.frag", defines);
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

  auto getBlur(const uint8_t extent, uint8_t blurDir, uint8_t blurDim)
  {
    Expects(extent > 0);
    Expects(blurDir < 3);
    Expects(blurDim > 0);
    Expects(blurDim < 3);
    return get("flat.vert",
               "blur.frag",
               {"BLUR_EXTENT " + std::to_string(extent),
                "BLUR_DIR " + std::to_string(blurDir),
                "BLUR_DIM " + std::to_string(blurDim)});
  }

  auto getVSMSquare()
  {
    return get("flat.vert", "vsm_square.frag");
  }

  auto getComposition()
  {
    return get("flat.vert", "composition.frag", {"LENS_DISTORTION"});
  }

  auto getCompositionWater()
  {
    return get("flat.vert", "composition.frag", {"WATER", "LENS_DISTORTION"});
  }

  auto getLightning()
  {
    return get("lightning.vert", "lightning.frag");
  }
};
} // namespace render::scene
