#pragma once

#include "mesh.h"
#include "render/material/material.h"
#include "render/material/materialmanager.h"
#include "render/material/shaderprogram.h"
#include "render/material/uniformparameter.h"
#include "rendercontext.h"

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

  explicit SingleBlur(std::string name,
                      material::MaterialManager& materialManager,
                      uint8_t dir,
                      uint8_t extent,
                      bool gauss,
                      int downscale)
      : m_name{std::move(name)}
      , m_material{gauss ? materialManager.getFastGaussBlur(extent, dir, PixelT::Channels)
                         : materialManager.getFastBoxBlur(extent, dir, PixelT::Channels)}
      , m_downscale{downscale}
  {
    Expects(dir == 1 || dir == 2);
    Expects(extent > 0);
    Expects(downscale > 0);
  }

  void setInput(const gslu::nn_shared<TextureHandle>& src)
  {
    m_blurredTexture = std::make_shared<TextureHandle>(
      gsl::make_shared<gl::Texture2D<PixelT>>(src->getTexture()->size() / m_downscale, m_name + "/blurred"),
      gsl::make_unique<gl::Sampler>(m_name + "/blurred-sampler")
        | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
        | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
        | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear));
    m_mesh = createScreenQuad(m_material, m_name + "/blur");
    m_mesh->bind("u_input",
                 [src](const Node* /*node*/, const Mesh& /*mesh*/, gl::Uniform& uniform)
                 {
                   uniform.set(src);
                 });

    m_framebuffer = gl::FrameBufferBuilder()
                      .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_blurredTexture->getTexture())
                      .build(m_name + "/framebuffer");
    m_mesh->getRenderState().merge(m_framebuffer->getRenderState());
    m_mesh->getRenderState().setViewport(src->getTexture()->size() / m_downscale);
  }

  void render() const
  {
    SOGLB_DEBUGGROUP(m_name + "/blur-pass");
    RenderContext context{material::RenderMode::Full, std::nullopt};
    m_framebuffer->bind();
    m_mesh->render(nullptr, context);
  }

  [[nodiscard]] gslu::nn_shared<TextureHandle> getBlurredTexture() const
  {
    return gsl::not_null{m_blurredTexture};
  }

private:
  const std::string m_name;
  std::shared_ptr<TextureHandle> m_blurredTexture;
  std::shared_ptr<Mesh> m_mesh;
  const std::shared_ptr<material::Material> m_material;
  std::shared_ptr<gl::Framebuffer> m_framebuffer;
  const int m_downscale;
};

template<typename PixelT>
class SeparableBlur
{
public:
  using TextureHandle = gl::TextureHandle<gl::Texture2D<PixelT>>;

  explicit SeparableBlur(const std::string& name,
                         material::MaterialManager& materialManager,
                         uint8_t extentX,
                         uint8_t extentY,
                         bool gauss,
                         int downscale = 1)
      : m_blur1{name + "/blur-1", materialManager, 1, extentX, gauss, downscale}
      , m_blur2{name + "/blur-2", materialManager, 2, extentY, gauss, 1}
  {
  }

  explicit SeparableBlur(const std::string& name,
                         material::MaterialManager& materialManager,
                         uint8_t extent,
                         bool gauss,
                         int downscale = 1,
                         const std::shared_ptr<TextureHandle>& src = nullptr)
      : SeparableBlur{name, materialManager, extent, extent, gauss, downscale}
  {
    if(src != nullptr)
      setInput(gsl::not_null{src});
  }

  void setInput(const gslu::nn_shared<TextureHandle>& src)
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
  // the first blur downscales, the second one works with the downscaled data
  SingleBlur<PixelT> m_blur1;
  SingleBlur<PixelT> m_blur2;
};
} // namespace render::scene
