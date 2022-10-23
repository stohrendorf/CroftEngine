#pragma once

#include "render/scene/blur.h"

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/fwd.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>

namespace render::material
{
class MaterialManager;
class Material;
} // namespace render::material

namespace render::scene
{
class Camera;
class Mesh;
} // namespace render::scene

namespace render::pass
{
class GeometryPass;

class EdgeDetectionPass
{
public:
  explicit EdgeDetectionPass(material::MaterialManager& materialManager,
                             const glm::ivec2& viewport,
                             const GeometryPass& geometryPass);

  void render();

  [[nodiscard]] const auto& getTexture() const
  {
    return m_dilationBufferHandle;
  }

private:
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
