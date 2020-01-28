#include "screenoverlay.h"

#include "material.h"
#include "mesh.h"
#include "renderer.h"
#include "shadermanager.h"
#include "uniformparameter.h"

#include <gl/image.h>

namespace render::scene
{
ScreenOverlay::ScreenOverlay(ShaderManager& shaderManager, const glm::ivec2& viewport)
{
  init(shaderManager, viewport);
}

ScreenOverlay::~ScreenOverlay() = default;

void ScreenOverlay::render(RenderContext& context)
{
  if(context.getRenderMode() != RenderMode::Full)
    return;

  context.pushState(getRenderState());
  m_texture->assign(m_image->getRawData());
  m_mesh->render(context);
  context.popState();
}

void ScreenOverlay::init(ShaderManager& shaderManager, const glm::ivec2& viewport)
{
  *m_image = gl::Image<gl::SRGBA8>(viewport);
  if(viewport.x == 0 || viewport.y == 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Cannot create screen overlay because the viewport is empty"));
  }

  const auto screenOverlayProgram = shaderManager.getScreenOverlay();

  m_texture = std::make_shared<gl::Texture2D<gl::SRGBA8>>(m_image->getSize());
  m_texture->assign(m_image->getRawData())
    .set(gl::api::TextureMinFilter::Nearest)
    .set(gl::api::TextureMagFilter::Nearest)
    .set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge);

  m_mesh = createQuadFullscreen(
    gsl::narrow<float>(viewport.x), gsl::narrow<float>(viewport.y), screenOverlayProgram->getHandle());
  m_mesh->getMaterial().set(RenderMode::Full, std::make_shared<Material>(screenOverlayProgram));
  m_mesh->getMaterial().get(RenderMode::Full)->getUniform("u_input")->set(m_texture);

  m_mesh->getRenderState().setCullFace(false);
  m_mesh->getRenderState().setDepthWrite(false);
  m_mesh->getRenderState().setDepthTest(false);
}
} // namespace render::scene
