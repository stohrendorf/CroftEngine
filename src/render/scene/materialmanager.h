#pragma once

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <utility>

namespace render::scene
{
class CSM;
class Camera;
class Material;
class Renderer;
class ShaderManager;

class MaterialManager final
{
public:
  explicit MaterialManager(gsl::not_null<std::shared_ptr<ShaderManager>> shaderManager,
                           gsl::not_null<std::shared_ptr<CSM>> csm,
                           gsl::not_null<std::shared_ptr<Renderer>> renderer);

  [[nodiscard]] const auto& getShaderManager() const
  {
    return m_shaderManager;
  }

  [[nodiscard]] const std::shared_ptr<Material>& getSprite();

  [[nodiscard]] const std::shared_ptr<Material>& getCSMDepthOnly(bool skeletal);
  [[nodiscard]] const std::shared_ptr<Material>& getDepthOnly(bool skeletal);

  [[nodiscard]] std::shared_ptr<Material> getGeometry(bool water, bool skeletal);

  [[nodiscard]] const std::shared_ptr<Material>& getPortal();

  [[nodiscard]] const std::shared_ptr<Material>& getLightning();

  [[nodiscard]] std::shared_ptr<Material> getComposition(bool water, bool lensDistortion, bool dof, bool filmGrain);

  [[nodiscard]] const std::shared_ptr<Material>& getCrt();

  void setGeometryTextures(std::shared_ptr<gl::Texture2DArray<gl::SRGBA8>> geometryTextures);

private:
  const gsl::not_null<std::shared_ptr<ShaderManager>> m_shaderManager;

  std::shared_ptr<Material> m_sprite{nullptr};
  std::array<std::shared_ptr<Material>, 2> m_csmDepthOnly{};
  std::array<std::shared_ptr<Material>, 2> m_depthOnly{};
  std::array<std::array<std::shared_ptr<Material>, 2>, 2> m_geometry{};
  std::shared_ptr<Material> m_portal{nullptr};
  std::shared_ptr<Material> m_lightning{nullptr};
  std::array<std::array<std::array<std::array<std::shared_ptr<Material>, 2>, 2>, 2>, 2> m_composition{};
  std::shared_ptr<Material> m_crt{nullptr};

  const gsl::not_null<std::shared_ptr<CSM>> m_csm;
  const gsl::not_null<std::shared_ptr<Renderer>> m_renderer;
  std::shared_ptr<gl::Texture2DArray<gl::SRGBA8>> m_geometryTextures;
};
} // namespace render::scene
