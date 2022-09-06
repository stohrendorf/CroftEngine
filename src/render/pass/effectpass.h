#pragma once

#include "config.h"
#include "render/renderpipeline.h"
#include "render/scene/mesh.h"
#include "render/scene/rendercontext.h"

#include <gl/debuggroup.h>
#include <gl/framebuffer.h>
#include <gl/sampler.h>
#include <gl/soglb_fwd.h>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <string>
#include <utility>

namespace render::scene
{
class Material;
class Mesh;
} // namespace render::scene

namespace render::pass
{
template<typename TPixel>
class EffectPass final
{
public:
  explicit EffectPass(gsl::not_null<const RenderPipeline*> renderPipeline,
                      std::string name,
                      const gslu::nn_shared<scene::Material>& material,
                      const gslu::nn_shared<gl::TextureHandle<gl::Texture2D<TPixel>>>& input)
      : m_name{std::move(name)}
      , m_mesh{scene::createScreenQuad(material, m_name)}
      , m_output{std::make_shared<gl::Texture2D<TPixel>>(input->getTexture()->size(), m_name + "-color")}
      , m_outputHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<TPixel>>>(
          m_output,
          gsl::make_unique<gl::Sampler>(m_name + "-color-sampler")
            | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
            | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
            | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear))}
      , m_fb{gl::FrameBufferBuilder()
               .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_output)
               .build(m_name + "-fb")}
  {
    m_mesh->bind("u_input",
                 [input](const scene::Node* /*node*/, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                 {
                   uniform.set(input);
                 });
    m_mesh->bind("u_time",
                 [renderPipeline
                  = std::move(renderPipeline)](const scene::Node*, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                 {
                   const auto now = renderPipeline->getLocalTime();
                   uniform.set(gsl::narrow_cast<float>(now.count()));
                 });

    m_mesh->getRenderState().merge(m_fb->getRenderState());
  }

  void render(bool inWater)
  {
    SOGLB_DEBUGGROUP(m_name + "-pass");

    m_fb->bind();

    scene::RenderContext context{scene::RenderMode::Full, std::nullopt};
    m_mesh->bind("u_inWater",
                 [inWater](const scene::Node* /*node*/, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                 {
                   uniform.set(inWater ? 1.0f : 0.0f);
                 });
    m_mesh->render(nullptr, context);

    if constexpr(FlushPasses)
      GL_ASSERT(gl::api::finish());
  }

  [[nodiscard]] const auto& getOutput() const
  {
    return m_outputHandle;
  }

  [[nodiscard]] const auto& getFramebuffer() const
  {
    return m_fb;
  }

  template<typename... Args>
  void bind(Args&&... args)
  {
    m_mesh->bind(std::forward<Args>(args)...);
  }

private:
  const std::string m_name;
  gslu::nn_shared<scene::Mesh> m_mesh;
  gslu::nn_shared<gl::Texture2D<TPixel>> m_output;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<TPixel>>> m_outputHandle;
  gslu::nn_shared<gl::Framebuffer> m_fb;
};
} // namespace render::pass
