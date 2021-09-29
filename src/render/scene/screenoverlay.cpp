#include "screenoverlay.h"

#include "materialmanager.h"
#include "mesh.h"
#include "rendercontext.h"
#include "rendermode.h"

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
#include <utility>

namespace render::scene
{
class Node;
}

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
  m_texture->getTexture()->assign(m_image->getData());
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

  auto texture = gslu::make_nn_shared<gl::Texture2D<gl::SRGBA8>>(m_image->getSize(), "screenoverlay");
  texture->assign(m_image->getData());
  m_texture = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>(
    texture,
    gslu::make_nn_unique<gl::Sampler>("screenoverlay-sampler") | set(gl::api::TextureMinFilter::Nearest)
      | set(gl::api::TextureMagFilter::Nearest)
      | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
      | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge));

  m_mesh = createScreenQuad(materialManager.getFlat(true, true), "screenoverlay");
  m_mesh->bind("u_input",
               [this](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               { uniform.set(gsl::not_null{m_texture}); });
  m_mesh->bind("u_alphaMultiplier",
               [this](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               { uniform.set(m_alphaMultiplier); });

  m_mesh->getRenderState().setBlend(0, true);
}
} // namespace render::scene
