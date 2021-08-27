#pragma once

#include <filesystem>
#include <gsl/gsl-lite.hpp>
#include <unordered_map>
#include <vector>

namespace render::scene
{
class ShaderProgram;

class ShaderCache final
{
  std::unordered_map<std::string, gsl::not_null<std::shared_ptr<ShaderProgram>>> m_programs{};

  const std::filesystem::path m_root;

public:
  explicit ShaderCache(std::filesystem::path root)
      : m_root{std::move(root)}
  {
  }

  static std::string makeId(const std::filesystem::path& vshPath,
                            const std::filesystem::path& fshPath,
                            const std::vector<std::string>& defines);

  gsl::not_null<std::shared_ptr<ShaderProgram>> get(const std::filesystem::path& vshPath,
                                                    const std::filesystem::path& fshPath,
                                                    const std::vector<std::string>& defines = {});

  auto getFlat(bool withAlphaMultiplier, bool invertY, bool withAspectRatio)
  {
    std::vector<std::string> defines;
    if(withAlphaMultiplier)
      defines.emplace_back("ALPHA_MULTIPLIER");
    if(invertY)
      defines.emplace_back("INVERT_Y");
    if(withAspectRatio)
      defines.emplace_back("ASPECT_RATIO");
    return get("flat.vert", "flat.frag", defines);
  }

  auto getBackdrop()
  {
    return get("backdrop.vert", "flat.frag");
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

  auto getWaterSurface()
  {
    return get("water_surface.vert", "water_surface.frag");
  }

  auto getFXAA()
  {
    return get("flat.vert", "fxaa.frag");
  }

  auto getHBAO()
  {
    return get("flat.vert", "hbao.frag");
  }

  auto getFastGaussBlur(const uint8_t extent, uint8_t blurDim)
  {
    Expects(extent > 0);
    Expects(blurDim > 0);
    Expects(blurDim < 3);
    std::vector<std::string> defines{"BLUR_DIM " + std::to_string(blurDim)};
    return get("flat.vert", "blur_fast_gauss_" + std::to_string(extent * 2 + 1) + ".frag", defines);
  }

  auto getFastBoxBlur(const uint8_t extent, uint8_t blurDim)
  {
    Expects(extent > 0);
    Expects(blurDim > 0);
    Expects(blurDim < 3);
    std::vector<std::string> defines{"BLUR_DIM " + std::to_string(blurDim)};
    return get("flat.vert", "blur_fast_box_" + std::to_string(extent * 2 + 1) + ".frag", defines);
  }

  auto getVSMSquare()
  {
    return get("flat.vert", "vsm_square.frag");
  }

  auto getComposition(bool water, bool lensDistortion, bool dof, bool filmGrain, bool hbao, bool velvia)
  {
    std::vector<std::string> defines;
    if(water)
      defines.emplace_back("WATER");
    if(lensDistortion)
      defines.emplace_back("LENS_DISTORTION");
    if(dof)
      defines.emplace_back("DOF");
    if(filmGrain)
      defines.emplace_back("FILM_GRAIN");
    if(hbao)
      defines.emplace_back("HBAO");
    if(velvia)
      defines.emplace_back("VELVIA");
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

  auto getLinearDepth()
  {
    return get("flat.vert", "linearize_depth.frag");
  }

  auto getUi()
  {
    return get("ui.vert", "ui.frag");
  }
};
} // namespace render::scene
