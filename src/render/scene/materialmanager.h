#pragma once

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <map>
#include <utility>

namespace render::scene
{
class CSM;
class Camera;
class Material;
class Renderer;
class ShaderCache;

class MaterialManager final
{
public:
  explicit MaterialManager(gsl::not_null<std::shared_ptr<ShaderCache>> shaderCache,
                           gsl::not_null<std::shared_ptr<Renderer>> renderer);

  [[nodiscard]] const std::shared_ptr<Material>& getSprite();

  [[nodiscard]] const std::shared_ptr<Material>& getCSMDepthOnly(bool skeletal);
  [[nodiscard]] const std::shared_ptr<Material>& getDepthOnly(bool skeletal);

  [[nodiscard]] std::shared_ptr<Material> getGeometry(bool water, bool skeletal, bool roomShadowing);

  [[nodiscard]] const std::shared_ptr<Material>& getWaterSurface();

  [[nodiscard]] const std::shared_ptr<Material>& getLightning();

  [[nodiscard]] std::shared_ptr<Material>
    getComposition(bool water, bool lensDistortion, bool dof, bool filmGrain, bool hbao);

  [[nodiscard]] const std::shared_ptr<Material>& getCrt();

  [[nodiscard]] const std::shared_ptr<Material>& getUi();

  [[nodiscard]] std::shared_ptr<Material> getFlat(bool withAlpha, bool invertY = false, bool withAspectRatio = false);
  [[nodiscard]] const std::shared_ptr<Material>& getBackdrop();
  [[nodiscard]] const std::shared_ptr<Material>& getFXAA();
  [[nodiscard]] const std::shared_ptr<Material>& getHBAO();
  [[nodiscard]] const std::shared_ptr<Material>& getLinearDepth();
  [[nodiscard]] const std::shared_ptr<Material>& getVSMSquare();
  [[nodiscard]] std::shared_ptr<Material> getFastGaussBlur(uint8_t extent, uint8_t blurDir, uint8_t blurDim);
  [[nodiscard]] std::shared_ptr<Material> getFastBoxBlur(uint8_t extent, uint8_t blurDir, uint8_t blurDim);

  void setGeometryTextures(std::shared_ptr<gl::TextureHandle<gl::Texture2DArray<gl::SRGBA8>>> geometryTextures);
  void setFiltering(bool bilinear, bool anisotropic);

  void setCSM(const gsl::not_null<std::shared_ptr<CSM>>& csm)
  {
    m_csm = csm;
  }

private:
  const gsl::not_null<std::shared_ptr<ShaderCache>> m_shaderCache;
  std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::RGB8>>> m_noiseTexture;

  std::shared_ptr<Material> m_sprite{nullptr};
  std::map<bool, std::shared_ptr<Material>> m_csmDepthOnly{};
  std::map<bool, std::shared_ptr<Material>> m_depthOnly{};
  std::map<std::tuple<bool, bool, bool>, std::shared_ptr<Material>> m_geometry{};
  std::shared_ptr<Material> m_waterSurface{nullptr};
  std::shared_ptr<Material> m_lightning{nullptr};
  std::map<std::tuple<bool, bool, bool, bool, bool>, std::shared_ptr<Material>> m_composition{};
  std::shared_ptr<Material> m_crt{nullptr};
  std::shared_ptr<Material> m_ui{nullptr};
  std::map<std::tuple<bool, bool, bool>, std::shared_ptr<Material>> m_flat{};
  std::map<std::tuple<uint8_t, uint8_t, uint8_t>, std::shared_ptr<Material>> m_fastGaussBlur{};
  std::map<std::tuple<uint8_t, uint8_t, uint8_t>, std::shared_ptr<Material>> m_fastBoxBlur{};
  std::shared_ptr<Material> m_backdrop{nullptr};
  std::shared_ptr<Material> m_fxaa{nullptr};
  std::shared_ptr<Material> m_hbao{nullptr};
  std::shared_ptr<Material> m_linearDepth{nullptr};
  std::shared_ptr<Material> m_vsmSquare{nullptr};

  std::shared_ptr<CSM> m_csm;
  const gsl::not_null<std::shared_ptr<Renderer>> m_renderer;
  std::shared_ptr<gl::TextureHandle<gl::Texture2DArray<gl::SRGBA8>>> m_geometryTextures;
};
} // namespace render::scene
