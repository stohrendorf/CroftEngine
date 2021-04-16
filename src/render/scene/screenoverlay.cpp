#include "screenoverlay.h"

#include "material.h"
#include "materialmanager.h"
#include "mesh.h"
#include "rendercontext.h"
#include "renderer.h"

#include <gl/texture2d.h>

namespace render::scene
{
ScreenOverlay::ScreenOverlay(MaterialManager& materialManager, const glm::ivec2& viewport)
{
  init(materialManager, viewport);
}

ScreenOverlay::~ScreenOverlay() = default;

bool ScreenOverlay::render(RenderContext& context)
{
  if(context.getRenderMode() != RenderMode::Full)
    return false;

  context.pushState(getRenderState());
  m_texture->assign(m_image->getRawData());
  m_mesh->render(context);
  context.popState();
  return true;
}

void ScreenOverlay::init(MaterialManager& materialManager, const glm::ivec2& viewport)
{
  *m_image = gl::Image<gl::SRGBA8>(viewport);
  if(viewport.x == 0 || viewport.y == 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Cannot create screen overlay because the viewport is empty"));
  }

  m_texture = std::make_shared<gl::Texture2D<gl::SRGBA8>>(m_image->getSize());
  m_texture->assign(m_image->getRawData())
    .set(gl::api::TextureMinFilter::Nearest)
    .set(gl::api::TextureMagFilter::Nearest)
    .set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge);

  m_mesh = createScreenQuad(materialManager.getFlat(true, true), "screenoverlay");
  m_mesh->bind("u_input",
               [this](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               { uniform.set(m_texture); });
  m_mesh->bind("u_alphaMultiplier",
               [this](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               { uniform.set(m_alphaMultiplier); });

  m_mesh->getRenderState().setBlend(true);
}
} // namespace render::scene
