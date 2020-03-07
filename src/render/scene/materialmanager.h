#pragma once

#include "material.h"
#include "renderer.h"
#include "shadermanager.h"

#include <gl/texture2darray.h>
#include <utility>

namespace render::scene
{
class CSM;
class Camera;

class MaterialManager final
{
public:
  explicit MaterialManager(gsl::not_null<std::shared_ptr<ShaderManager>> shaderManager,
                           gsl::not_null<std::shared_ptr<CSM>> csm,
                           gsl::not_null<std::shared_ptr<Renderer>> renderer);

  [[nodiscard]] auto& getShaderManager() const
  {
    return m_shaderManager;
  }

  [[nodiscard]] auto& getShaderManager()
  {
    return m_shaderManager;
  }

  [[nodiscard]] const std::shared_ptr<Material>& getSprite();

  [[nodiscard]] const std::shared_ptr<Material>& getCSMDepthOnly();
  [[nodiscard]] const std::shared_ptr<Material>& getDepthOnly();

  [[nodiscard]] std::shared_ptr<Material>
    createMaterial(const gsl::not_null<std::shared_ptr<gl::Texture2DArray<gl::SRGBA8>>>& texture, bool water);

  [[nodiscard]] const std::shared_ptr<Material>& getPortal();

  [[nodiscard]] const std::shared_ptr<Material>& getLightning();

private:
  const gsl::not_null<std::shared_ptr<ShaderManager>> m_shaderManager;

  std::shared_ptr<Material> m_sprite{nullptr};
  std::shared_ptr<Material> m_csmDepthOnly{nullptr};
  std::shared_ptr<Material> m_depthOnly{nullptr};
  std::shared_ptr<Material> m_materialWater{nullptr};
  std::shared_ptr<Material> m_material{nullptr};
  std::shared_ptr<Material> m_portal{nullptr};
  std::shared_ptr<Material> m_lightning{nullptr};

  const gsl::not_null<std::shared_ptr<CSM>> m_csm;
  const gsl::not_null<std::shared_ptr<Renderer>> m_renderer;
};
} // namespace render::scene
