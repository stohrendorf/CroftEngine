#include "screenoverlay.h"

#include "material.h"
#include "mesh.h"
#include "render/gl/image.h"
#include "renderer.h"
#include "shadermanager.h"
#include "uniformparameter.h"

namespace render::scene
{
ScreenOverlay::ScreenOverlay(ShaderManager& shaderManager, const Dimension2<size_t>& viewport)
{
  init(shaderManager, viewport);
}

ScreenOverlay::~ScreenOverlay() = default;

void ScreenOverlay::render(RenderContext& context)
{
  if(context.getRenderMode() != RenderMode::Full)
    return;

  context.pushState(getRenderState());
  m_texture->image(m_image->getData());
  m_mesh->render(context);
  context.popState();
}

void ScreenOverlay::init(ShaderManager& shaderManager, const Dimension2<size_t>& viewport)
{
  *m_image = gl::Image<gl::SRGBA8>(gsl::narrow<int32_t>(viewport.width), gsl::narrow<int32_t>(viewport.height));
  if(viewport.width == 0 || viewport.height == 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Cannot create screen overlay because the viewport is empty"));
  }

  const auto screenOverlayProgram = shaderManager.get("screenoverlay.vert", "screenoverlay.frag", {});

  m_texture->image(m_image->getWidth(), m_image->getHeight(), m_image->getData());
  m_texture->set(::gl::TextureMinFilter::Nearest)
    .set(::gl::TextureMagFilter::Nearest)
    .set(::gl::TextureParameterName::TextureWrapS, ::gl::TextureWrapMode::ClampToEdge)
    .set(::gl::TextureParameterName::TextureWrapT, ::gl::TextureWrapMode::ClampToEdge);

  m_mesh = createQuadFullscreen(
    gsl::narrow<float>(viewport.width), gsl::narrow<float>(viewport.height), screenOverlayProgram->getHandle(), true);
  m_mesh->setMaterial(std::make_shared<Material>(screenOverlayProgram), RenderMode::Full);
  m_mesh->getMaterial(RenderMode::Full)->getUniform("u_texture")->set(m_texture.get());
  m_mesh->getMaterial(RenderMode::Full)
    ->getUniform("u_projection")
    ->set(glm::ortho(0.0f, gsl::narrow<float>(viewport.width), gsl::narrow<float>(viewport.height), 0.0f, 0.0f, 1.0f));

  m_mesh->getRenderState().setCullFace(false);
  m_mesh->getRenderState().setDepthWrite(false);
  m_mesh->getRenderState().setDepthTest(false);
}
} // namespace render::scene
