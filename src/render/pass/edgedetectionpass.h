#pragma once

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/vec2.hpp>
#include <gslu.h>

namespace render::material
{
class MaterialManager;
}

namespace render::scene
{
class Mesh;
}

namespace render::pass
{
class GeometryPass;

class EdgeDetectionPass
{
public:
  explicit EdgeDetectionPass(material::MaterialManager& materialManager,
                             const glm::ivec2& viewport,
                             const gslu::nn_shared<GeometryPass>& geometryPass);

  void render();

  [[nodiscard]] const auto& getTexture() const
  {
    return m_dilationBufferHandle;
  }

private:
  gslu::nn_shared<GeometryPass> m_geometryPass;
  gslu::nn_shared<scene::Mesh> m_edgeRenderMesh;
  gslu::nn_shared<gl::Texture2D<gl::ScalarByte>> m_edgeBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::ScalarByte>>> m_edgeBufferHandle;
  gslu::nn_shared<gl::Framebuffer> m_edgeFb;

  gslu::nn_shared<scene::Mesh> m_dilationRenderMesh;
  gslu::nn_shared<gl::Texture2D<gl::ScalarByte>> m_dilationBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::ScalarByte>>> m_dilationBufferHandle;
  gslu::nn_shared<gl::Framebuffer> m_dilationFb;
};
} // namespace render::pass
