#pragma once

#include <filesystem>
#include <gsl-lite.hpp>
#include <unordered_map>

namespace render::scene
{
class ShaderProgram;

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

  auto getFlat(bool withAlphaMultiplier, bool invertY = true)
  {
    std::vector<std::string> defines;
    if(withAlphaMultiplier)
      defines.emplace_back("ALPHA_MULTIPLIER");
    if(invertY)
      defines.emplace_back("INVERT_Y");
    return get("flat.vert", "flat.frag", defines);
  }

  auto getGeometry(bool water, bool skeletal, bool roomShadowing)
  {
    std::vector<std::string> defines;
    if(water)
      defines.emplace_back("WATER");
    if(skeletal)
      defines.emplace_back("SKELETAL");
    if(roomShadowing)
      defines.emplace_back("ROOM_SHADOWING");
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
    return get("depth_only.vert", "depth_only.frag", defines);
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

  auto getBlur(const uint8_t extent, uint8_t blurDir, uint8_t blurDim, bool gauss, bool fillGaps)
  {
    Expects(extent > 0);
    Expects(blurDir < 3);
    Expects(blurDim > 0);
    Expects(blurDim < 3);
    std::vector<std::string> defines{"BLUR_EXTENT " + std::to_string(extent),
                                     "BLUR_DIR " + std::to_string(blurDir),
                                     "BLUR_DIM " + std::to_string(blurDim)};
    if(gauss)
      defines.emplace_back("BLUR_GAUSS");
    if(fillGaps)
      defines.emplace_back("FILL_GAPS");
    return get("flat.vert", "blur.frag", defines);
  }

  auto getVSMSquare()
  {
    return get("flat.vert", "vsm_square.frag");
  }

  auto getComposition(bool water, bool lensDistortion, bool dof, bool filmGrain)
  {
    std::vector<std::string> defines;
    if(water)
      defines.emplace_back("WATER");
    if(lensDistortion)
      defines.emplace_back("LENS_DISTORTION");
    if(dof)
      defines.emplace_back("DOF");
    if(filmGrain)
      defines.emplace_back("GILM_GRAIN");
    return get("flat.vert", "composition.frag", defines);
  }

  auto getLightning()
  {
    return get("lightning.vert", "lightning.frag");
  }

  auto getCrt()
  {
    return get("flat.vert", "crt.frag");
  }

  auto getScreenSpriteTextured()
  {
    return get("screensprite.vert", "screensprite.frag", {"SCREENSPRITE_TEXTURE"});
  }

  auto getScreenSpriteColorRect()
  {
    return get("screensprite.vert", "screensprite.frag");
  }
};
} // namespace render::scene
