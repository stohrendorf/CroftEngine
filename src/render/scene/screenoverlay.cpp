#include "screenoverlay.h"

#include "material.h"
#include "mesh.h"
#include "rendercontext.h"
#include "renderer.h"
#include "shadermanager.h"
#include "uniformparameter.h"

#include <gl/image.h>
#include <gl/texture2d.h>

namespace render::scene
{
ScreenOverlay::ScreenOverlay(ShaderManager& shaderManager, const glm::ivec2& viewport)
{
  init(shaderManager, viewport);
}

ScreenOverlay::~ScreenOverlay() = default;

bool ScreenOverlay::render(RenderContext& context)
{
  if(context.getRenderMode() != RenderMode::Full)
    return false;

  context.pushState(getRenderState());
  m_texture->assign(m_image->getRawData());
  m_mesh->getMaterial().get(RenderMode::Full)->getUniform("u_alphaMultiplier")->set(m_alphaMultiplier);
  m_mesh->render(context);
  context.popState();
  return true;
}

void ScreenOverlay::init(ShaderManager& shaderManager, const glm::ivec2& viewport)
{
  *m_image = gl::Image<gl::SRGBA8>(viewport);
  if(viewport.x == 0 || viewport.y == 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Cannot create screen overlay because the viewport is empty"));
  }

  const auto screenOverlayProgram = shaderManager.getFlat(true);

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
  m_mesh->getRenderState().setBlend(true);
}
} // namespace render::scene
