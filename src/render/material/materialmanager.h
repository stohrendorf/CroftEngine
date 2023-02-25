#pragma once

#include <cstdint>
#include <functional>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <map>
#include <memory>
#include <optional>
#include <tuple>

namespace render::scene
{
class CSM;
class Renderer;
} // namespace render::scene

namespace render::material
{
class Material;
class ShaderCache;
enum class SpriteMaterialMode : uint8_t;

class MaterialManager final
{
public:
  explicit MaterialManager(gslu::nn_shared<ShaderCache> shaderCache, gslu::nn_shared<scene::Renderer> renderer);

  [[nodiscard]] gslu::nn_shared<Material> getSprite(SpriteMaterialMode mode, std::function<int32_t()> lightingMode);

  [[nodiscard]] gslu::nn_shared<Material> getCSMDepthOnly(bool skeletal, std::function<bool()> smooth);
  [[nodiscard]] gslu::nn_shared<Material> getDepthOnly(bool skeletal, std::function<bool()> smooth);

  [[nodiscard]] gslu::nn_shared<Material> getGeometry(bool inWater,
                                                      bool skeletal,
                                                      bool roomShadowing,
                                                      std::function<bool()> smooth,
                                                      std::function<int32_t()> lightingMode);
  [[nodiscard]] gslu::nn_shared<Material> getGhost(std::function<bool()> smooth);

  [[nodiscard]] gslu::nn_shared<Material> getWaterSurface();

  [[nodiscard]] gslu::nn_shared<Material> getLightning();

  [[nodiscard]] gslu::nn_shared<Material> getWorldComposition(bool inWater, bool dof);

  [[nodiscard]] gslu::nn_shared<Material> getUi();

  [[nodiscard]] gslu::nn_shared<Material> getDustParticle();

  [[nodiscard]] gslu::nn_shared<Material> getGhostName();

  [[nodiscard]] gslu::nn_shared<Material> getFXAA(uint8_t preset);
  [[nodiscard]] gslu::nn_shared<Material> getCRTV0();
  [[nodiscard]] gslu::nn_shared<Material> getCRTV1();
  [[nodiscard]] gslu::nn_shared<Material> getBrightnessContrast(int8_t brightness, int8_t contrast);
  [[nodiscard]] gslu::nn_shared<Material> getVelvia();
  [[nodiscard]] gslu::nn_shared<Material> getDeath();
  [[nodiscard]] gslu::nn_shared<Material> getFilmGrain();
  [[nodiscard]] gslu::nn_shared<Material> getLensDistortion();
  [[nodiscard]] gslu::nn_shared<Material> getMasking(bool ao, bool edges);
  [[nodiscard]] gslu::nn_shared<Material> getUnderwaterMovement();
  [[nodiscard]] gslu::nn_shared<Material> getReflective();
  [[nodiscard]] gslu::nn_shared<Material> getBloom();

  [[nodiscard]] gslu::nn_shared<Material> getFlat(bool withAlpha, bool invertY = false, bool withAspectRatio = false);
  [[nodiscard]] gslu::nn_shared<Material> getBackdrop(bool withAlphaMultiplier);
  [[nodiscard]] gslu::nn_shared<Material> getHBAO();
  [[nodiscard]] gslu::nn_shared<Material> getEdgeDetection();
  [[nodiscard]] gslu::nn_shared<Material> getEdgeDilation();
  [[nodiscard]] gslu::nn_shared<Material> getVSMSquare();
  [[nodiscard]] gslu::nn_shared<Material> getFastGaussBlur(uint8_t extent, uint8_t blurDir, uint8_t blurDim);
  [[nodiscard]] gslu::nn_shared<Material> getFastBoxBlur(uint8_t extent, uint8_t blurDir, uint8_t blurDim);
  [[nodiscard]] gslu::nn_shared<Material> getBloomDownsample();
  [[nodiscard]] gslu::nn_shared<Material> getBloomUpsample();

  void setGeometryTextures(const gslu::nn_shared<gl::Texture2DArray<gl::PremultipliedSRGBA8>>& geometryTextures);
  void setFiltering(bool bilinear, const std::optional<float>& anisotropyLevel);

  void setCSM(const gslu::nn_shared<scene::CSM>& csm)
  {
    m_csm = csm;
  }

  void setDeathStrength(float strength);

private:
  const gslu::nn_shared<ShaderCache> m_shaderCache;
  std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::RGB8>>> m_noiseTexture;

  std::map<uint8_t, gslu::nn_shared<Material>> m_fxaa{};
  std::shared_ptr<Material> m_crtV0{nullptr};
  std::shared_ptr<Material> m_crtV1{nullptr};
  std::map<std::tuple<int8_t, int8_t>, gslu::nn_shared<Material>> m_brightnessContrast{};
  std::shared_ptr<Material> m_velvia{nullptr};
  std::shared_ptr<Material> m_death{nullptr};
  std::shared_ptr<Material> m_filmGrain{nullptr};
  std::shared_ptr<Material> m_lensDistortion{nullptr};
  std::map<std::tuple<bool, bool>, gslu::nn_shared<Material>> m_masking{};
  std::shared_ptr<Material> m_underwaterMovement{nullptr};
  std::shared_ptr<Material> m_reflective{nullptr};
  std::shared_ptr<Material> m_bloom{nullptr};

  std::map<SpriteMaterialMode, gslu::nn_shared<Material>> m_sprite{};
  std::shared_ptr<Material> m_ghost{nullptr};
  std::shared_ptr<Material> m_waterSurface{nullptr};
  std::shared_ptr<Material> m_lightning{nullptr};
  std::map<std::tuple<bool, bool>, gslu::nn_shared<Material>> m_composition{};
  std::shared_ptr<Material> m_ui{nullptr};
  std::map<std::tuple<bool, bool, bool>, gslu::nn_shared<Material>> m_flat{};
  std::map<std::tuple<uint8_t, uint8_t, uint8_t>, gslu::nn_shared<Material>> m_fastGaussBlur{};
  std::map<std::tuple<uint8_t, uint8_t, uint8_t>, gslu::nn_shared<Material>> m_fastBoxBlur{};
  std::shared_ptr<Material> m_bloomDownsample{};
  std::shared_ptr<Material> m_bloomUpsample{};
  std::map<bool, gslu::nn_shared<Material>> m_backdrop{};
  std::shared_ptr<Material> m_hbao{nullptr};
  std::shared_ptr<Material> m_edgeDetection{nullptr};
  std::shared_ptr<Material> m_edgeDilation{nullptr};
  std::shared_ptr<Material> m_vsmSquare{nullptr};

  std::shared_ptr<Material> m_dustParticle{nullptr};
  std::shared_ptr<Material> m_ghostName{nullptr};

  std::shared_ptr<scene::CSM> m_csm;
  const gslu::nn_shared<scene::Renderer> m_renderer;
  std::shared_ptr<gl::TextureHandle<gl::Texture2DArray<gl::PremultipliedSRGBA8>>> m_geometryTexturesHandle;

  void createSampler(const gslu::nn_shared<gl::Texture2DArray<gl::PremultipliedSRGBA8>>& geometryTextures,
                     bool bilinear,
                     const std::optional<float>& anisotropyLevel);
};
} // namespace render::material
