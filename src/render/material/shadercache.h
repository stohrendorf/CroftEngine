#pragma once

#include <cstdint>
#include <filesystem>
#include <gsl-lite/gsl-lite.hpp>
#include <gslu.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace render::material
{
class ShaderProgram;

class ShaderCache final
{
  std::unordered_map<std::string, gslu::nn_shared<ShaderProgram>> m_programs;

  std::filesystem::path m_root;

public:
  explicit ShaderCache(std::filesystem::path root)
      : m_root{std::move(root)}
  {
  }

  [[nodiscard]] gslu::nn_shared<ShaderProgram> get(const std::filesystem::path& vshPath,
                                                   const std::filesystem::path& fshPath,
                                                   const std::vector<std::string>& defines = {});

  [[nodiscard]] gslu::nn_shared<ShaderProgram> get(const std::filesystem::path& vshPath,
                                                   const std::filesystem::path& fshPath,
                                                   const std::filesystem::path& geomPath,
                                                   const std::vector<std::string>& defines = {});

  [[nodiscard]] auto getFlat(const bool withAlphaMultiplier, const bool invertY, const bool withAspectRatio)
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

  [[nodiscard]] auto getBackdrop(const bool withAlphaMultiplier)
  {
    std::vector<std::string> defines;
    if(withAlphaMultiplier)
      defines.emplace_back("ALPHA_MULTIPLIER");
    return get("backdrop.vert", "flat.frag", defines);
  }

  [[nodiscard]] auto getGeometry(
    const bool inWater, const bool skeletal, const bool roomShadowing, const bool opaque, const uint8_t spriteMode)
  {
    std::vector<std::string> defines;
    if(inWater)
      defines.emplace_back("IN_WATER");
    if(skeletal)
      defines.emplace_back("SKELETAL");
    if(roomShadowing)
      defines.emplace_back("ROOM_SHADOWING");
    if(opaque)
      defines.emplace_back("EARLY_FRAGMENT_TEST");
    defines.emplace_back("SPRITEMODE " + std::to_string(static_cast<int>(spriteMode)));
    return get("geometry.vert", "geometry.frag", defines);
  }

  [[nodiscard]] auto getCSMDepthOnly(const bool skeletal)
  {
    std::vector<std::string> defines;
    if(skeletal)
      defines.emplace_back("SKELETAL");
    return get("csm_depth_only.vert", "empty.frag", defines);
  }

  [[nodiscard]] auto getDepthOnly(const bool skeletal)
  {
    std::vector<std::string> defines;
    if(skeletal)
      defines.emplace_back("SKELETAL");
    return get("depth_only.vert", "depth_only.frag", defines);
  }

  [[nodiscard]] auto getWaterSurface()
  {
    return get("water_surface.vert", "water_surface.frag");
  }

  [[nodiscard]] auto getFXAA(const uint8_t preset)
  {
    return get(
      "flat.vert", "fx_fxaa.frag", std::vector{"FXAA_QUALITY__PRESET " + std::to_string(static_cast<int>(preset))});
  }

  [[nodiscard]] auto getCRTV0()
  {
    return get("flat.vert", "fx_crt_v0.frag");
  }

  [[nodiscard]] auto getCRTV1()
  {
    return get("flat.vert", "fx_crt_v1.frag");
  }

  [[nodiscard]] auto getBrightnessContrast(const int8_t brightness, const int8_t contrast)
  {
    return get("flat.vert",
               "fx_brightness_contrast.frag",
               std::vector{"BRIGHTNESS " + std::to_string(static_cast<int>(brightness)),
                           "CONTRAST " + std::to_string(static_cast<int>(contrast))});
  }

  [[nodiscard]] auto getVelvia()
  {
    return get("flat.vert", "fx_velvia.frag");
  }

  [[nodiscard]] auto getDeath()
  {
    return get("flat.vert", "fx_death.frag");
  }

  [[nodiscard]] auto getFilmGrain()
  {
    return get("flat.vert", "fx_film_grain.frag");
  }

  [[nodiscard]] auto getLensDistortion()
  {
    return get("flat.vert", "fx_lens_distortion.frag");
  }

  [[nodiscard]] auto getMasking(const bool ao, const bool edges)
  {
    std::vector<std::string> defines;
    if(ao)
      defines.emplace_back("AO");
    if(edges)
      defines.emplace_back("EDGES");
    return get("flat.vert", "fx_masking.frag", defines);
  }

  [[nodiscard]] auto getUnderwaterMovement()
  {
    return get("flat.vert", "fx_underwater_movement.frag");
  }

  [[nodiscard]] auto getHBAO()
  {
    return get("flat.vert", "hbao.frag");
  }

  [[nodiscard]] auto getEdgeDetection()
  {
    return get("flat.vert", "edge_detection.frag");
  }

  [[nodiscard]] auto getEdgeDilation()
  {
    return get("flat.vert", "edge_dilation.frag");
  }

  [[nodiscard]] auto getReflective()
  {
    return get("flat.vert", "fx_reflective.frag");
  }

  [[nodiscard]] auto getBloom()
  {
    return get("flat.vert", "fx_bloom.frag");
  }

  [[nodiscard]] auto getFastGaussBlur(const uint8_t extent, const uint8_t blurDim)
  {
    gsl_Expects(extent > 0);
    gsl_Expects(blurDim > 0);
    gsl_Expects(blurDim <= 3);
    const std::vector defines{"BLUR_DIM " + std::to_string(blurDim)};
    return get("flat.vert", "blur_fast_gauss_" + std::to_string(extent * 2 + 1) + ".frag", defines);
  }

  [[nodiscard]] auto getFastBoxBlur(const uint8_t extent, const uint8_t blurDim)
  {
    gsl_Expects(extent > 0);
    gsl_Expects(blurDim > 0);
    gsl_Expects(blurDim <= 3);
    const std::vector defines{"BLUR_DIM " + std::to_string(blurDim)};
    return get("flat.vert", "blur_fast_box_" + std::to_string(extent * 2 + 1) + ".frag", defines);
  }

  [[nodiscard]] auto getBloomDownsample()
  {
    return get("flat.vert", "bloom_downsample.frag");
  }

  [[nodiscard]] auto getBloomUpsample()
  {
    return get("flat.vert", "bloom_upsample.frag");
  }

  [[nodiscard]] auto getVSMSquare()
  {
    return get("flat.vert", "vsm_square.frag");
  }

  [[nodiscard]] auto getWorldComposition(const bool inWater, const bool dof)
  {
    std::vector<std::string> defines;
    if(inWater)
      defines.emplace_back("IN_WATER");
    if(dof)
      defines.emplace_back("DOF");
    return get("flat.vert", "composition.frag", defines);
  }

  [[nodiscard]] auto getLightning()
  {
    return get("lightning.vert", "lightning.frag");
  }

  [[nodiscard]] auto getUi()
  {
    return get("ui.vert", "ui.frag");
  }

  [[nodiscard]] auto getDustParticle()
  {
    return get("dust.vert", "dust.frag");
  }

  [[nodiscard]] auto getGhost()
  {
    return get("ghost.vert", "ghost.frag", std::vector<std::string>{"SKELETAL"});
  }

  [[nodiscard]] auto getGhostName()
  {
    std::vector<std::string> defines;
    defines.emplace_back("SPRITEMODE 2");
    return get("geometry.vert", "ghost_name.frag", defines);
  }
};
} // namespace render::material