#include "screenoverlay.h"

#include "mesh.h"
#include "render/material/materialmanager.h"
#include "render/material/rendermode.h"
#include "rendercontext.h"

#include <boost/throw_exception.hpp>
#include <gl/image.h>
#include <gl/pixel.h>
#include <gl/program.h>
#include <gl/renderstate.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texturehandle.h>
#include <glm/vec2.hpp>
#include <gslu.h>
#include <stdexcept>

namespace render::scene
{
class Node;
}

namespace render::scene
{
ScreenOverlay::ScreenOverlay() = default;
ScreenOverlay::~ScreenOverlay() = default;

void ScreenOverlay::render(const Node* node, RenderContext& context)
{
  if(context.getRenderMode() != material::RenderMode::Full)
    return;

  context.pushState(getRenderState());
  m_texture->getTexture()->assign(m_image->getData());
  m_mesh->render(node, context);
  context.popState();
}

void ScreenOverlay::init(material::MaterialManager& materialManager, const glm::ivec2& viewport)
{
  *m_image = gl::Image<gl::PremultipliedSRGBA8>(viewport);
  if(viewport.x == 0 || viewport.y == 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Cannot create screen overlay because the viewport is empty"));
  }

  auto texture = gsl::make_shared<gl::Texture2D<gl::PremultipliedSRGBA8>>(m_image->getSize(), "screenoverlay");
  texture->assign(m_image->getData());
  m_texture = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::PremultipliedSRGBA8>>>(
    texture,
    gsl::make_unique<gl::Sampler>("screenoverlay-sampler") | set(gl::api::TextureMinFilter::Nearest)
      | set(gl::api::TextureMagFilter::Nearest)
      | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
      | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge));

  m_mesh = createScreenQuad(materialManager.getFlat(true, true), "screenoverlay");
  m_mesh->bind("u_input",
               [this](const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               {
                 uniform.set(gsl::not_null{m_texture});
               });
  m_mesh->bind("u_alphaMultiplier",
               [this](const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               {
                 uniform.set(m_alphaMultiplier);
               });

  m_mesh->getRenderState().setBlend(0, true);
  m_mesh->getRenderState().setViewport(viewport);
}

void ScreenOverlay::render(const Node* /*node*/, RenderContext& /*context*/, gl::api::core::SizeType /*instanceCount*/)
{
  BOOST_THROW_EXCEPTION(std::runtime_error("Cannot render instanced ScreenOverlay"));
}
} // namespace render::scene
