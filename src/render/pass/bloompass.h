#pragma once

#include "config.h"
#include "render/renderpipeline.h"
#include "render/scene/blur.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "render/scene/rendercontext.h"

#include <array>
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
} // namespace render::scene

namespace render::pass
{
template<typename TPixel>
class SingleBloomDownsample final
{
public:
  using TextureHandle = gl::TextureHandle<gl::Texture2D<TPixel>>;

  explicit SingleBloomDownsample(const std::string& name,
                                 scene::MaterialManager& materialManager,
                                 const gslu::nn_shared<TextureHandle>& input)
      : m_name{name}
      , m_mesh{scene::createScreenQuad(materialManager.getBloomDownsample(), m_name)}
      , m_output{std::make_shared<gl::Texture2D<TPixel>>(input->getTexture()->size() / 2, m_name + "-color")}
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

    m_mesh->getRenderState().merge(m_fb->getRenderState());
  }

  void render()
  {
    SOGLB_DEBUGGROUP(m_name + "-downsample");

    m_fb->bind();

    scene::RenderContext context{scene::RenderMode::Full, std::nullopt};
    m_mesh->render(nullptr, context);

    if constexpr(FlushPasses)
      GL_ASSERT(gl::api::finish());
  }

  [[nodiscard]] const auto& getOutput() const
  {
    return m_outputHandle;
  }

private:
  std::string m_name;
  gslu::nn_shared<scene::Mesh> m_mesh;
  gslu::nn_shared<gl::Texture2D<TPixel>> m_output;
  gslu::nn_shared<TextureHandle> m_outputHandle;
  gslu::nn_shared<gl::Framebuffer> m_fb;
};

template<uint8_t NSteps, typename TPixel>
class BloomDownsample final
{
  static_assert(NSteps > 0);

public:
  using TextureHandle = gl::TextureHandle<gl::Texture2D<TPixel>>;

  explicit BloomDownsample(const std::string& name,
                           scene::MaterialManager& materialManager,
                           gslu::nn_shared<TextureHandle> input)
  {
    for(uint8_t i = 0; i < NSteps; ++i)
    {
      m_steps[i]
        = std::make_shared<SingleBloomDownsample<TPixel>>(name + ":" + std::to_string(int(i)), materialManager, input);
      input = m_steps[i]->getOutput();
    }
  }

  void render()
  {
    for(const auto& step : m_steps)
      step->render();
  }

  [[nodiscard]] const auto& getOutput() const
  {
    return m_steps.back()->getOutput();
  }

private:
  std::array<std::shared_ptr<SingleBloomDownsample<TPixel>>, NSteps> m_steps{};
};

template<typename TPixel>
class SingleBloomUpsample final
{
public:
  using TextureHandle = gl::TextureHandle<gl::Texture2D<TPixel>>;

  explicit SingleBloomUpsample(const std::string& name,
                               scene::MaterialManager& materialManager,
                               const gslu::nn_shared<TextureHandle>& input)
      : m_name{name}
      , m_mesh{scene::createScreenQuad(materialManager.getBloomUpsample(), m_name)}
      , m_output{std::make_shared<gl::Texture2D<TPixel>>(input->getTexture()->size() * 2, m_name + "-color")}
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

    m_mesh->getRenderState().merge(m_fb->getRenderState());
  }

  void render()
  {
    SOGLB_DEBUGGROUP(m_name + "-upsample");

    m_fb->bind();

    scene::RenderContext context{scene::RenderMode::Full, std::nullopt};
    m_mesh->render(nullptr, context);

    if constexpr(FlushPasses)
      GL_ASSERT(gl::api::finish());
  }

  [[nodiscard]] const auto& getOutput() const
  {
    return m_outputHandle;
  }

private:
  std::string m_name;
  gslu::nn_shared<scene::Mesh> m_mesh;
  gslu::nn_shared<gl::Texture2D<TPixel>> m_output;
  gslu::nn_shared<TextureHandle> m_outputHandle;
  gslu::nn_shared<gl::Framebuffer> m_fb;
};

template<uint8_t NSteps, typename TPixel>
class BloomUpsample final
{
  static_assert(NSteps > 0);

public:
  using TextureHandle = gl::TextureHandle<gl::Texture2D<TPixel>>;

  explicit BloomUpsample(const std::string& name,
                         scene::MaterialManager& materialManager,
                         gslu::nn_shared<TextureHandle> input)
  {
    for(uint8_t i = 0; i < NSteps; ++i)
    {
      m_steps[i]
        = std::make_shared<SingleBloomUpsample<TPixel>>(name + ":" + std::to_string(int(i)), materialManager, input);
      input = m_steps[i]->getOutput();
    }
  }

  void render()
  {
    for(const auto& step : m_steps)
      step->render();
  }

  [[nodiscard]] const auto& getOutput() const
  {
    return m_steps.back()->getOutput();
  }

private:
  std::array<std::shared_ptr<SingleBloomUpsample<TPixel>>, NSteps> m_steps{};
};

template<uint8_t NSteps, typename TPixel>
class BloomPass final
{
public:
  using TextureHandle = gl::TextureHandle<gl::Texture2D<TPixel>>;

  explicit BloomPass(scene::MaterialManager& materialManager, gslu::nn_shared<TextureHandle> input)
      : m_downsample{"bloom", materialManager, input}
      , m_blur{"bloom-blur", materialManager, 2, false, 1, m_downsample.getOutput()}
      , m_upsample{"bloom", materialManager, m_blur.getBlurredTexture()}
  {
  }

  void render()
  {
    m_downsample.render();
    m_blur.render();
    m_upsample.render();
  }

  [[nodiscard]] const auto& getOutput() const
  {
    return m_upsample.getOutput();
  }

private:
  BloomDownsample<NSteps, TPixel> m_downsample;
  scene::SeparableBlur<TPixel> m_blur;
  BloomUpsample<NSteps, TPixel> m_upsample;
};
} // namespace render::pass
