#pragma once

#include <cstdint>
#include <filesystem>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace render::scene
{
class ShaderProgram;

class ShaderCache final
{
  std::unordered_map<std::string, gslu::nn_shared<ShaderProgram>> m_programs{};

  const std::filesystem::path m_root;

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

  [[nodiscard]] auto getFlat(bool withAlphaMultiplier, bool invertY, bool withAspectRatio)
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

  [[nodiscard]] auto getBackdrop(bool withAlphaMultiplier)
  {
    std::vector<std::string> defines;
    if(withAlphaMultiplier)
      defines.emplace_back("ALPHA_MULTIPLIER");
    return get("backdrop.vert", "flat.frag", defines);
  }

  [[nodiscard]] auto getGeometry(bool inWater, bool skeletal, bool roomShadowing, uint8_t spriteMode)
  {
    std::vector<std::string> defines;
    if(inWater)
      defines.emplace_back("IN_WATER");
    if(skeletal)
      defines.emplace_back("SKELETAL");
    if(roomShadowing)
      defines.emplace_back("ROOM_SHADOWING");
    defines.emplace_back("SPRITEMODE " + std::to_string(int(spriteMode)));
    return get("geometry.vert", "geometry.frag", defines);
  }

  [[nodiscard]] auto getCSMDepthOnly(bool skeletal)
  {
    std::vector<std::string> defines;
    if(skeletal)
      defines.emplace_back("SKELETAL");
    return get("csm_depth_only.vert", "empty.frag", defines);
  }

  [[nodiscard]] auto getDepthOnly(bool skeletal)
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

  [[nodiscard]] auto getFXAA(uint8_t preset)
  {
    return get(
      "flat.vert", "fx_fxaa.frag", std::vector<std::string>{"FXAA_QUALITY__PRESET " + std::to_string(int(preset))});
  }

  [[nodiscard]] auto getCRTV0()
  {
    return get("flat.vert", "fx_crt_v0.frag");
  }

  [[nodiscard]] auto getCRTV1()
  {
    return get("flat.vert", "fx_crt_v1.frag");
  }

  [[nodiscard]] auto getBrightnessContrast(int8_t brightness, int8_t contrast)
  {
    return get("flat.vert",
               "fx_brightness_contrast.frag",
               std::vector<std::string>{"BRIGHTNESS " + std::to_string(int(brightness)),
                                        "CONTRAST " + std::to_string(int(contrast))});
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

  [[nodiscard]] auto getMasking(bool ao, bool edges)
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

  [[nodiscard]] auto getBloomFilter()
  {
    return get("flat.vert", "fx_bloom_filter.frag");
  }

  [[nodiscard]] auto getFastGaussBlur(const uint8_t extent, uint8_t blurDim)
  {
    Expects(extent > 0);
    Expects(blurDim > 0);
    Expects(blurDim <= 3);
    std::vector<std::string> defines{"BLUR_DIM " + std::to_string(blurDim)};
    return get("flat.vert", "blur_fast_gauss_" + std::to_string(extent * 2 + 1) + ".frag", defines);
  }

  [[nodiscard]] auto getFastBoxBlur(const uint8_t extent, uint8_t blurDim)
  {
    Expects(extent > 0);
    Expects(blurDim > 0);
    Expects(blurDim < 3);
    std::vector<std::string> defines{"BLUR_DIM " + std::to_string(blurDim)};
    return get("flat.vert", "blur_fast_box_" + std::to_string(extent * 2 + 1) + ".frag", defines);
  }

  [[nodiscard]] auto getVSMSquare()
  {
    return get("flat.vert", "vsm_square.frag");
  }

  [[nodiscard]] auto getWorldComposition(bool inWater, bool dof)
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
    return get("dust.vert", "dust.frag", "dust.geom");
  }

  [[nodiscard]] auto getGhost()
  {
    return get("ghost.vert", "ghost.frag", std::vector<std::string>{"SKELETAL"});
  }
};
} // namespace render::scene
