#pragma once

#include "material.h"
#include "materialmanager.h"
#include "mesh.h"
#include "rendercontext.h"
#include "shaderprogram.h"
#include "uniformparameter.h"

#include <gl/debuggroup.h>
#include <gl/framebuffer.h>
#include <gl/sampler.h>
#include <gl/texturehandle.h>
#include <gslu.h>

namespace render::scene
{
template<typename PixelT>
class SingleBlur
{
public:
  using TextureHandle = gl::TextureHandle<gl::Texture2D<PixelT>>;

  explicit SingleBlur(std::string name, MaterialManager& materialManager, uint8_t dir, uint8_t extent, bool gauss)
      : m_name{std::move(name)}
      , m_material{gauss ? materialManager.getFastGaussBlur(extent, dir, PixelT::Channels)
                         : materialManager.getFastBoxBlur(extent, dir, PixelT::Channels)}
  {
    Expects(dir == 1 || dir == 2);
    Expects(extent > 0);
  }

  void setInput(const gsl::not_null<std::shared_ptr<TextureHandle>>& src)
  {
    m_blurredTexture = std::make_shared<TextureHandle>(
      gslu::make_nn_shared<gl::Texture2D<PixelT>>(src->getTexture()->size(), m_name + "/blurred"),
      gslu::make_nn_unique<gl::Sampler>(m_name + "/blurred")
        | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
        | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
        | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear));
    m_mesh = createScreenQuad(m_material, m_name + "/blur");
    m_mesh->bind("u_input",
                 [src](const Node& /*node*/, const Mesh& /*mesh*/, gl::Uniform& uniform) { uniform.set(src); });

    m_framebuffer = gl::FrameBufferBuilder()
                      .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_blurredTexture->getTexture())
                      .build(m_name + "/framebuffer");
    m_mesh->getRenderState().merge(m_framebuffer->getRenderState());
  }

  void render() const
  {
    SOGLB_DEBUGGROUP(m_name + "/blur-pass");
    RenderContext context{RenderMode::Full, std::nullopt};
    m_framebuffer->bind();
    m_mesh->render(context);
  }

  [[nodiscard]] gsl::not_null<std::shared_ptr<TextureHandle>> getBlurredTexture() const
  {
    return gsl::not_null{m_blurredTexture};
  }

private:
  const std::string m_name;
  std::shared_ptr<TextureHandle> m_blurredTexture;
  std::shared_ptr<Mesh> m_mesh;
  const std::shared_ptr<Material> m_material;
  std::shared_ptr<gl::Framebuffer> m_framebuffer;
};

template<typename PixelT>
class SeparableBlur
{
public:
  using TextureHandle = gl::TextureHandle<gl::Texture2D<PixelT>>;

  explicit SeparableBlur(const std::string& name, MaterialManager& materialManager, uint8_t extent, bool gauss)
      : m_blur1{name + "/blur-1", materialManager, 1, extent, gauss}
      , m_blur2{name + "/blur-2", materialManager, 2, extent, gauss}
  {
  }

  void setInput(const gsl::not_null<std::shared_ptr<TextureHandle>>& src)
  {
    m_blur1.setInput(src);
    m_blur2.setInput(m_blur1.getBlurredTexture());
  }

  void render()
  {
    m_blur1.render();
    m_blur2.render();
  }

  [[nodiscard]] auto getBlurredTexture() const
  {
    return m_blur2.getBlurredTexture();
  }

private:
  SingleBlur<PixelT> m_blur1;
  SingleBlur<PixelT> m_blur2;
};
} // namespace render::scene
