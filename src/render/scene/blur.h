#pragma once

#include "material.h"
#include "model.h"
#include "shadermanager.h"
#include "shaderprogram.h"

#include <gl/framebuffer.h>
#include <gl/texture2d.h>

namespace render::scene
{
template<typename _PixelT, uint8_t _Dir, uint8_t _Extent>
class SingleBlur
{
  static_assert(_Dir == 1 || _Dir == 2);
  static_assert(_Extent > 0);

public:
  using Texture = gl::Texture2D<_PixelT>;

  explicit SingleBlur(std::string name, ShaderManager& shaderManager)
      : m_name{std::move(name)}
      , m_shader{shaderManager.getBlur(_Extent, _Dir, _PixelT::Channels)}
      , m_material{std::make_shared<Material>(m_shader)}
  {
    m_model->getRenderState().setCullFace(false);
    m_model->getRenderState().setDepthTest(false);
    m_model->getRenderState().setDepthWrite(false);
  }

  void resize(const glm::ivec2& size, const std::shared_ptr<Texture>& src)
  {
    m_blurredTexture = std::make_shared<Texture>(size, m_name + "/blurred");
    m_blurredTexture->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
      .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
      .set(gl::api::TextureMinFilter::Linear)
      .set(gl::api::TextureMagFilter::Linear);

    m_material->getUniform("u_input")->set(src);

    m_model->getMeshes().clear();
    m_model->addMesh(
      createQuadFullscreen(gsl::narrow<float>(size.x), gsl::narrow<float>(size.y), m_shader->getHandle()));
    m_model->getMeshes()[0]->getMaterial().set(RenderMode::Full, m_material);

    m_framebuffer = gl::FrameBufferBuilder()
                      .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_blurredTexture)
                      .build(m_name + "/framebuffer");
  }

  void render(const glm::ivec2& size) const
  {
    gl::DebugGroup dbg{m_name + "/blur-pass"};
    GL_ASSERT(gl::api::viewport(0, 0, size.x, size.y));

    gl::RenderState state;
    state.setBlend(false);
    state.apply(true);
    RenderContext context{RenderMode::Full, std::nullopt};
    Node dummyNode{""};
    context.setCurrentNode(&dummyNode);

    m_framebuffer->bindWithAttachments();
    m_model->render(context);
  }

  void invalidate()
  {
    m_framebuffer->invalidate();
  }

  const std::shared_ptr<Texture>& getBlurredTexture() const
  {
    return m_blurredTexture;
  }

private:
  const std::string m_name;
  std::shared_ptr<Texture> m_blurredTexture;
  const std::shared_ptr<Model> m_model = std::make_shared<scene::Model>();
  const std::shared_ptr<ShaderProgram> m_shader;
  const std::shared_ptr<Material> m_material;
  std::shared_ptr<gl::Framebuffer> m_framebuffer;
};

template<typename _PixelT, uint8_t _Extent>
class SeparableBlur
{
public:
  using Texture = gl::Texture2D<_PixelT>;

  explicit SeparableBlur(const std::string& name, ShaderManager& shaderManager)
      : m_blur1{name + "/blur-1", shaderManager}
      , m_blur2{name + "/blur-2", shaderManager}
  {
  }

  void resize(const glm::ivec2& size, const std::shared_ptr<Texture>& src)
  {
    m_blur1.resize(size, src);
    m_blur2.resize(size, m_blur1.getBlurredTexture());
  }

  void render(const glm::ivec2& size)
  {
    m_blur1.render(size);
    m_blur2.render(size);
    m_blur1.invalidate();
  }

  void invalidate()
  {
    m_blur2.invalidate();
  }

  const std::shared_ptr<Texture>& getBlurredTexture() const
  {
    return m_blur2.getBlurredTexture();
  }

private:
  SingleBlur<_PixelT, 1, _Extent> m_blur1;
  SingleBlur<_PixelT, 2, _Extent> m_blur2;
};
} // namespace render::scene