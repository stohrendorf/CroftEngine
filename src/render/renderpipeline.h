#pragma once

#include "gl/debuggroup.h"
#include "gl/framebuffer.h"
#include "gl/texture.h"
#include "render/scene/dimension.h"
#include "render/scene/material.h"
#include "scene/model.h"
#include "scene/rendercontext.h"

#include <memory>
#include <random>

namespace render
{
class RenderPipeline
{
  const std::shared_ptr<scene::ShaderProgram> m_fxaaShader
    = scene::ShaderProgram::createFromFile("shaders/flat.vert", "shaders/fxaa.frag", {});
  const std::shared_ptr<scene::Material> m_fxaaMaterial = std::make_shared<scene::Material>(m_fxaaShader);
  const std::shared_ptr<scene::ShaderProgram> m_ssaoShader
    = scene::ShaderProgram::createFromFile("shaders/flat.vert", "shaders/ssao.frag", {});
  const std::shared_ptr<scene::Material> m_ssaoMaterial = std::make_shared<scene::Material>(m_ssaoShader);
  const std::shared_ptr<scene::ShaderProgram> m_ssaoBlurShader
    = scene::ShaderProgram::createFromFile("shaders/flat.vert", "shaders/ssao_blur.frag", {});
  const std::shared_ptr<scene::Material> m_ssaoBlurMaterial = std::make_shared<scene::Material>(m_ssaoBlurShader);
  const std::shared_ptr<scene::ShaderProgram> m_fxDarknessShader
    = scene::ShaderProgram::createFromFile("shaders/flat.vert", "shaders/fx_darkness.frag", {"LENS_DISTORTION"});
  const std::shared_ptr<scene::Material> m_fxDarknessMaterial = std::make_shared<scene::Material>(m_fxDarknessShader);
  const std::shared_ptr<scene::ShaderProgram> m_fxWaterDarknessShader = scene::ShaderProgram::createFromFile(
    "shaders/flat.vert", "shaders/fx_darkness.frag", {"WATER", "LENS_DISTORTION"});
  const std::shared_ptr<scene::Material> m_fxWaterDarknessMaterial
    = std::make_shared<scene::Material>(m_fxWaterDarknessShader);

  const std::shared_ptr<scene::Model> m_fbModel = std::make_shared<scene::Model>();

  const std::shared_ptr<gl::TextureDepth> m_portalDepthBuffer = std::make_shared<gl::TextureDepth>("portal-depth");
  const std::shared_ptr<gl::Texture2D<gl::RG32F>> m_portalPerturbBuffer
    = std::make_shared<gl::Texture2D<gl::RG32F>>("portal-perturb");
  std::shared_ptr<gl::Framebuffer> m_portalFb;

  const std::shared_ptr<gl::TextureDepth> m_geometryDepthBuffer = std::make_shared<gl::TextureDepth>("geometry-depth");
  const std::shared_ptr<gl::Texture2D<gl::SRGBA8>> m_geometryColorBuffer
    = std::make_shared<gl::Texture2D<gl::SRGBA8>>("geometry-color");
  const std::shared_ptr<gl::Texture2D<gl::RGB32F>> m_geometryPositionBuffer
    = std::make_shared<gl::Texture2D<gl::RGB32F>>("geometry-position");
  const std::shared_ptr<gl::Texture2D<gl::RGB16F>> m_geometryNormalBuffer
    = std::make_shared<gl::Texture2D<gl::RGB16F>>("geometry-normal");
  std::shared_ptr<gl::Framebuffer> m_geometryFb;

  const std::shared_ptr<gl::Texture2D<gl::RGB32F>> m_ssaoNoiseTexture
    = std::make_shared<gl::Texture2D<gl::RGB32F>>("ssao-noise");
  const std::shared_ptr<gl::Texture2D<gl::Scalar32F>> m_ssaoAOBuffer
    = std::make_shared<gl::Texture2D<gl::Scalar32F>>("ssao-ao");
  std::shared_ptr<gl::Framebuffer> m_ssaoFb;

  const std::shared_ptr<gl::Texture2D<gl::Scalar32F>> m_ssaoBlurAOBuffer
    = std::make_shared<gl::Texture2D<gl::Scalar32F>>("ssao-blur-ao");
  std::shared_ptr<gl::Framebuffer> m_ssaoBlurFb;

  const std::shared_ptr<gl::Texture2D<gl::SRGBA8>> m_fxaaColorBuffer
    = std::make_shared<gl::Texture2D<gl::SRGBA8>>("fxaa-color");
  std::shared_ptr<gl::Framebuffer> m_fxaaFb;

public:
  // ReSharper disable once CppMemberFunctionMayBeConst
  void bindGeometryFrameBuffer()
  {
    m_geometryFb->bind();
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void bindPortalFrameBuffer()
  {
    gl::Framebuffer::unbindAll();
    m_portalDepthBuffer->copyImageSubData(*m_geometryDepthBuffer);
    m_portalFb->bind();
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void finalPass(const bool water)
  {
    {
      gl::DebugGroup dbg{"ssao-pass"};
      m_ssaoFb->bind();
      scene::RenderContext context{};
      scene::Node dummyNode{""};
      context.setCurrentNode(&dummyNode);

      m_fbModel->getMeshes()[0]->setMaterial(m_ssaoMaterial);
      m_fbModel->render(context);

      m_ssaoBlurFb->bind();
      m_fbModel->getMeshes()[0]->setMaterial(m_ssaoBlurMaterial);
      m_fbModel->render(context);
    }

    {
      gl::DebugGroup dbg{"fxaa-pass"};
      m_fxaaFb->bind();
      scene::RenderContext context{};
      scene::Node dummyNode{""};
      context.setCurrentNode(&dummyNode);

      m_fbModel->getMeshes()[0]->setMaterial(m_fxaaMaterial);
      m_fbModel->render(context);
    }

    {
      gl::DebugGroup dbg{"postprocess-pass"};
      gl::Framebuffer::unbindAll();
      if(water)
        m_fbModel->getMeshes()[0]->setMaterial(m_fxWaterDarknessMaterial);
      else
        m_fbModel->getMeshes()[0]->setMaterial(m_fxDarknessMaterial);
      scene::RenderContext context{};
      scene::Node dummyNode{""};
      context.setCurrentNode(&dummyNode);
      m_fbModel->render(context);
    }
  }

  explicit RenderPipeline(const scene::Dimension2<size_t>& viewport)
  {
    resize(viewport);
    /*
         * [geometry] --> geometryFB --- depth --> (copy) -------------> portalDepthFB --- depth ---------------------> fx_darkness.glsl --> @viewport
         *                           `         `  [portal geometry] --´                                              ´
         *                           `         `---------------------------------------------------------------------´
         *                           `-- color --> fxaa.glsl --> fxaaFB ---------------------------------------------´
         *                           `-- normals ---> ssao.glsl --> ssaoFB --> ssaoBlur.glsl --> ssaoBlurFB --> AO --´
         *                           `-- position --´
         */
    // === geometryFB setup ===
    m_geometryColorBuffer->set(::gl::TextureParameterName::TextureWrapS, ::gl::TextureWrapMode::ClampToEdge)
      .set(::gl::TextureParameterName::TextureWrapT, ::gl::TextureWrapMode::ClampToEdge)
      .set(::gl::TextureMinFilter::Linear)
      .set(::gl::TextureMagFilter::Linear);
    m_fxaaMaterial->getUniform("u_texture")->set(m_geometryColorBuffer);

    m_geometryNormalBuffer->set(::gl::TextureParameterName::TextureWrapS, ::gl::TextureWrapMode::ClampToEdge)
      .set(::gl::TextureParameterName::TextureWrapT, ::gl::TextureWrapMode::ClampToEdge)
      .set(::gl::TextureMinFilter::Nearest)
      .set(::gl::TextureMagFilter::Nearest);
    m_ssaoMaterial->getUniform("u_normals")->set(m_geometryNormalBuffer);

    m_geometryPositionBuffer->set(::gl::TextureParameterName::TextureWrapS, ::gl::TextureWrapMode::ClampToEdge)
      .set(::gl::TextureParameterName::TextureWrapT, ::gl::TextureWrapMode::ClampToEdge)
      .set(::gl::TextureMinFilter::Nearest)
      .set(::gl::TextureMagFilter::Nearest);
    m_ssaoMaterial->getUniform("u_position")->set(m_geometryPositionBuffer);

    m_fxDarknessMaterial->getUniform("u_depth")->set(m_geometryDepthBuffer);
    m_fxWaterDarknessMaterial->getUniform("u_depth")->set(m_geometryDepthBuffer);

    m_geometryFb = gl::FrameBufferBuilder()
                     .texture(::gl::FramebufferAttachment::ColorAttachment0, m_geometryColorBuffer)
                     .texture(::gl::FramebufferAttachment::ColorAttachment1, m_geometryNormalBuffer)
                     .texture(::gl::FramebufferAttachment::ColorAttachment2, m_geometryPositionBuffer)
                     .texture(::gl::FramebufferAttachment::DepthAttachment, m_geometryDepthBuffer)
                     .build();

    // === portalDepthFB setup ===
    m_fxDarknessMaterial->getUniform("u_portalDepth")->set(m_portalDepthBuffer);
    m_fxDarknessMaterial->getUniform("u_portalPerturb")->set(m_portalPerturbBuffer);
    m_fxWaterDarknessMaterial->getUniform("u_portalDepth")->set(m_portalDepthBuffer);
    m_fxWaterDarknessMaterial->getUniform("u_portalPerturb")->set(m_portalPerturbBuffer);

    m_portalFb = gl::FrameBufferBuilder()
                   .texture(::gl::FramebufferAttachment::DepthAttachment, m_portalDepthBuffer)
                   .texture(::gl::FramebufferAttachment::ColorAttachment0, m_portalPerturbBuffer)
                   .build();

    // === fxaaFB setup ===
    m_fxaaColorBuffer->set(::gl::TextureParameterName::TextureWrapS, ::gl::TextureWrapMode::ClampToEdge)
      .set(::gl::TextureParameterName::TextureWrapT, ::gl::TextureWrapMode::ClampToEdge)
      .set(::gl::TextureMinFilter::Linear)
      .set(::gl::TextureMagFilter::Linear);
    m_fxDarknessMaterial->getUniform("u_texture")->set(m_fxaaColorBuffer);
    m_fxWaterDarknessMaterial->getUniform("u_texture")->set(m_fxaaColorBuffer);

    m_fxaaFb
      = gl::FrameBufferBuilder().texture(::gl::FramebufferAttachment::ColorAttachment0, m_fxaaColorBuffer).build();

    // === ssaoFB setup ===
    m_ssaoAOBuffer->set(::gl::TextureParameterName::TextureWrapS, ::gl::TextureWrapMode::ClampToEdge)
      .set(::gl::TextureParameterName::TextureWrapT, ::gl::TextureWrapMode::ClampToEdge)
      .set(::gl::TextureMinFilter::Linear)
      .set(::gl::TextureMagFilter::Linear);
    m_ssaoBlurMaterial->getUniform("u_ao")->set(m_ssaoAOBuffer);

    m_ssaoFb = gl::FrameBufferBuilder().texture(::gl::FramebufferAttachment::ColorAttachment0, m_ssaoAOBuffer).build();

    // === ssaoBlurFB setup ===
    m_ssaoBlurAOBuffer->set(::gl::TextureParameterName::TextureWrapS, ::gl::TextureWrapMode::ClampToEdge)
      .set(::gl::TextureParameterName::TextureWrapT, ::gl::TextureWrapMode::ClampToEdge)
      .set(::gl::TextureMinFilter::Linear)
      .set(::gl::TextureMagFilter::Linear);
    m_fxDarknessMaterial->getUniform("u_ao")->set(m_ssaoBlurAOBuffer);
    m_fxWaterDarknessMaterial->getUniform("u_ao")->set(m_ssaoBlurAOBuffer);

    m_ssaoBlurFb
      = gl::FrameBufferBuilder().texture(::gl::FramebufferAttachment::ColorAttachment0, m_ssaoBlurAOBuffer).build();

    // === ssao.glsl setup ===
    // generate sample kernel
    std::uniform_real_distribution<float> randomFloats(0, 1);
    std::default_random_engine generator{}; // NOLINT(cert-msc32-c)
    std::vector<glm::vec3> ssaoSamples;
    while(ssaoSamples.size() < 64)
    {
#define SSAO_UNIFORM_VOLUME_SAMPLING
#ifdef SSAO_SAMPLE_CONTRACTION
      glm::vec3 sample{randomFloats(generator) * 2 - 1, randomFloats(generator) * 2 - 1, randomFloats(generator)};
      sample = glm::normalize(sample) * randomFloats(generator);
      // scale samples s.t. they're more aligned to center of kernel
      const float scale = float(i) / 64;
      ssaoSamples.emplace_back(sample * glm::mix(0.1f, 1.0f, scale * scale));
#elif defined(SSAO_UNIFORM_VOLUME_SAMPLING)
      glm::vec3 sample{randomFloats(generator) * 2 - 1, randomFloats(generator) * 2 - 1, randomFloats(generator)};
      if(length(sample) > 1)
        continue;
      ssaoSamples.emplace_back(sample);
#else
      glm::vec3 sample{randomFloats(generator) * 2 - 1, randomFloats(generator) * 2 - 1, randomFloats(generator)};
      sample = glm::normalize(sample) * randomFloats(generator);
      ssaoSamples.emplace_back(sample);
#endif
    }
    m_ssaoMaterial->getUniform("u_samples[0]")->set(ssaoSamples);

    // generate noise texture
    std::vector<gl::RGB32F> ssaoNoise;
    ssaoNoise.reserve(16);
    for(int i = 0; i < 16; ++i)
    {
      // rotate around z-axis (in tangent space)
      ssaoNoise.emplace_back(randomFloats(generator) * 2 - 1, randomFloats(generator) * 2 - 1, 0.0f);
    }

    m_ssaoNoiseTexture->image(4, 4, ssaoNoise)
      .set(::gl::TextureParameterName::TextureWrapS, ::gl::TextureWrapMode::Repeat)
      .set(::gl::TextureParameterName::TextureWrapT, ::gl::TextureWrapMode::Repeat)
      .set(::gl::TextureMinFilter::Nearest)
      .set(::gl::TextureMagFilter::Nearest);
    m_ssaoMaterial->getUniform("u_texNoise")->set(m_ssaoNoiseTexture);

    m_fxDarknessMaterial->getUniform("distortion_power")->set(-1.0f);
    m_fxWaterDarknessMaterial->getUniform("distortion_power")->set(-2.0f);

    m_fbModel->getRenderState().setCullFace(false);
    m_fbModel->getRenderState().setDepthTest(false);
    m_fbModel->getRenderState().setDepthWrite(false);
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void update(const scene::Camera& camera, const std::chrono::high_resolution_clock::time_point& time)
  {
    // update uniforms
    const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(time);
    m_fxDarknessMaterial->getUniform("u_time")->set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
    m_fxWaterDarknessMaterial->getUniform("u_time")->set(gsl::narrow_cast<float>(now.time_since_epoch().count()));

    m_fxDarknessMaterial->getUniform("aspect_ratio")->set(camera.getAspectRatio());
    m_fxWaterDarknessMaterial->getUniform("aspect_ratio")->set(camera.getAspectRatio());

    m_fxDarknessMaterial->getUniform("u_camProjection")->set(camera.getProjectionMatrix());
    m_fxWaterDarknessMaterial->getUniform("u_camProjection")->set(camera.getProjectionMatrix());
    m_ssaoMaterial->getUniform("u_camProjection")->set(camera.getProjectionMatrix());
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void resize(const scene::Dimension2<size_t>& viewport)
  {
    m_portalDepthBuffer->image(gsl::narrow<int32_t>(viewport.width), gsl::narrow<int32_t>(viewport.height))
      .set(::gl::TextureMinFilter::Linear)
      .set(::gl::TextureMagFilter::Linear);
    m_portalPerturbBuffer->image(gsl::narrow<int32_t>(viewport.width), gsl::narrow<int32_t>(viewport.height))
      .set(::gl::TextureMinFilter::Linear)
      .set(::gl::TextureMagFilter::Linear);
    m_geometryDepthBuffer->image(gsl::narrow<int32_t>(viewport.width), gsl::narrow<int32_t>(viewport.height))
      .set(::gl::TextureMinFilter::Linear)
      .set(::gl::TextureMagFilter::Linear);
    m_geometryColorBuffer->image(gsl::narrow<int32_t>(viewport.width), gsl::narrow<int32_t>(viewport.height));
    m_geometryPositionBuffer->image(gsl::narrow<int32_t>(viewport.width), gsl::narrow<int32_t>(viewport.height));
    m_geometryNormalBuffer->image(gsl::narrow<int32_t>(viewport.width), gsl::narrow<int32_t>(viewport.height));
    m_ssaoAOBuffer->image(gsl::narrow<int32_t>(viewport.width), gsl::narrow<int32_t>(viewport.height));
    m_ssaoBlurAOBuffer->image(gsl::narrow<int32_t>(viewport.width), gsl::narrow<int32_t>(viewport.height));
    m_fxaaColorBuffer->image(gsl::narrow<int32_t>(viewport.width), gsl::narrow<int32_t>(viewport.height));

    const auto proj
      = glm::ortho(0.0f, gsl::narrow<float>(viewport.width), gsl::narrow<float>(viewport.height), 0.0f, 0.0f, 1.0f);

    m_fxaaShader->findUniform("u_projection")->set(proj);
    m_ssaoShader->findUniform("u_projection")->set(proj);
    m_ssaoBlurShader->findUniform("u_projection")->set(proj);
    m_fxDarknessShader->findUniform("u_projection")->set(proj);
    m_fxWaterDarknessShader->findUniform("u_projection")->set(proj);

    auto fxaaMesh = scene::createQuadFullscreen(
      gsl::narrow<float>(viewport.width), gsl::narrow<float>(viewport.height), m_fxaaShader->getHandle());
    fxaaMesh->setMaterial(m_fxaaMaterial);

    m_fbModel->getMeshes().clear();
    m_fbModel->addMesh(fxaaMesh);
  }
};
} // namespace render
