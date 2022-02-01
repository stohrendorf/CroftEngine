#pragma once

#include <cstdint>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <map>
#include <memory>
#include <tuple>

namespace render::scene
{
class CSM;
class Material;
class Renderer;
class ShaderCache;

class MaterialManager final
{
public:
  explicit MaterialManager(gslu::nn_shared<ShaderCache> shaderCache, gslu::nn_shared<Renderer> renderer);

  [[nodiscard]] gslu::nn_shared<Material> getSprite(bool billboard);

  [[nodiscard]] gslu::nn_shared<Material> getCSMDepthOnly(bool skeletal);
  [[nodiscard]] gslu::nn_shared<Material> getDepthOnly(bool skeletal);

  [[nodiscard]] gslu::nn_shared<Material> getGeometry(bool inWater, bool skeletal, bool roomShadowing);
  [[nodiscard]] gslu::nn_shared<Material> getGhost();

  [[nodiscard]] gslu::nn_shared<Material> getWaterSurface();

  [[nodiscard]] gslu::nn_shared<Material> getLightning();

  [[nodiscard]] gslu::nn_shared<Material> getWorldComposition(bool inWater, bool dof);

  [[nodiscard]] gslu::nn_shared<Material> getUi();

  [[nodiscard]] gslu::nn_shared<Material> getDustParticle();

  [[nodiscard]] gslu::nn_shared<Material> getFXAA();
  [[nodiscard]] gslu::nn_shared<Material> getCRT();
  [[nodiscard]] gslu::nn_shared<Material> getVelvia();
  [[nodiscard]] gslu::nn_shared<Material> getFilmGrain();
  [[nodiscard]] gslu::nn_shared<Material> getLensDistortion();
  [[nodiscard]] gslu::nn_shared<Material> getHBAOFx();
  [[nodiscard]] gslu::nn_shared<Material> getUnderwaterMovement();

  [[nodiscard]] gslu::nn_shared<Material> getFlat(bool withAlpha, bool invertY = false, bool withAspectRatio = false);
  [[nodiscard]] const std::shared_ptr<Material>& getBackdrop();
  [[nodiscard]] gslu::nn_shared<Material> getHBAO();
  [[nodiscard]] gslu::nn_shared<Material> getVSMSquare();
  [[nodiscard]] gslu::nn_shared<Material> getFastGaussBlur(uint8_t extent, uint8_t blurDir, uint8_t blurDim);
  [[nodiscard]] gslu::nn_shared<Material> getFastBoxBlur(uint8_t extent, uint8_t blurDir, uint8_t blurDim);

  void setGeometryTextures(std::shared_ptr<gl::TextureHandle<gl::Texture2DArray<gl::SRGBA8>>> geometryTextures);
  void setFiltering(bool bilinear, float anisotropyLevel);

  void setCSM(const gslu::nn_shared<CSM>& csm)
  {
    m_csm = csm;
  }

private:
  const gslu::nn_shared<ShaderCache> m_shaderCache;
  std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::RGB8>>> m_noiseTexture;

  std::shared_ptr<Material> m_fxaa{nullptr};
  std::shared_ptr<Material> m_crt{nullptr};
  std::shared_ptr<Material> m_velvia{nullptr};
  std::shared_ptr<Material> m_filmGrain{nullptr};
  std::shared_ptr<Material> m_lensDistortion{nullptr};
  std::shared_ptr<Material> m_hbaoFx{nullptr};
  std::shared_ptr<Material> m_underwaterMovement{nullptr};

  std::map<bool, gslu::nn_shared<Material>> m_sprite{};
  std::map<bool, gslu::nn_shared<Material>> m_csmDepthOnly{};
  std::map<bool, gslu::nn_shared<Material>> m_depthOnly{};
  std::map<std::tuple<bool, bool, bool>, gslu::nn_shared<Material>> m_geometry{};
  std::shared_ptr<Material> m_ghost{nullptr};
  std::shared_ptr<Material> m_waterSurface{nullptr};
  std::shared_ptr<Material> m_lightning{nullptr};
  std::map<std::tuple<bool, bool>, gslu::nn_shared<Material>> m_composition{};
  std::shared_ptr<Material> m_ui{nullptr};
  std::map<std::tuple<bool, bool, bool>, gslu::nn_shared<Material>> m_flat{};
  std::map<std::tuple<uint8_t, uint8_t, uint8_t>, gslu::nn_shared<Material>> m_fastGaussBlur{};
  std::map<std::tuple<uint8_t, uint8_t, uint8_t>, gslu::nn_shared<Material>> m_fastBoxBlur{};
  std::shared_ptr<Material> m_backdrop{nullptr};
  std::shared_ptr<Material> m_hbao{nullptr};
  std::shared_ptr<Material> m_vsmSquare{nullptr};

  std::shared_ptr<Material> m_dustParticle{nullptr};

  std::shared_ptr<CSM> m_csm;
  const gslu::nn_shared<Renderer> m_renderer;
  std::shared_ptr<gl::TextureHandle<gl::Texture2DArray<gl::SRGBA8>>> m_geometryTextures;
};
} // namespace render::scene
