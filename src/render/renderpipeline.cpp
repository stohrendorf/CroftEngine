#include "renderpipeline.h"

#include "scene/mesh.h"
#include "scene/node.h"
#include "scene/shadermanager.h"

#include <random>

namespace render
{
RenderPipeline::RenderPipeline(scene::ShaderManager& shaderManager, const scene::Dimension2<size_t>& viewport)
    : m_fxaaShader{shaderManager.get("flat.vert", "fxaa.frag", {})}
    , m_fxaaMaterial{std::make_shared<scene::Material>(m_fxaaShader)}
    , m_ssaoShader{shaderManager.get("flat.vert", "ssao.frag", {})}
    , m_ssaoMaterial{std::make_shared<scene::Material>(m_ssaoShader)}
    , m_ssaoBlurShader{shaderManager.get("flat.vert", "ssao_blur.frag", {})}
    , m_ssaoBlurMaterial{std::make_shared<scene::Material>(m_ssaoBlurShader)}
    , m_fxDarknessShader{shaderManager.get("flat.vert", "fx_darkness.frag", {"LENS_DISTORTION"})}
    , m_fxDarknessMaterial{std::make_shared<scene::Material>(m_fxDarknessShader)}
    , m_fxWaterDarknessShader{shaderManager.get("flat.vert", "fx_darkness.frag", {"WATER", "LENS_DISTORTION"})}
    , m_fxWaterDarknessMaterial{std::make_shared<scene::Material>(m_fxWaterDarknessShader)}
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
                   .textureNoBlend(::gl::FramebufferAttachment::ColorAttachment1, m_geometryNormalBuffer)
                   .textureNoBlend(::gl::FramebufferAttachment::ColorAttachment2, m_geometryPositionBuffer)
                   .textureNoBlend(::gl::FramebufferAttachment::DepthAttachment, m_geometryDepthBuffer)
                   .build();

  // === portalDepthFB setup ===
  m_fxDarknessMaterial->getUniform("u_portalDepth")->set(m_portalDepthBuffer);
  m_fxDarknessMaterial->getUniform("u_portalPerturb")->set(m_portalPerturbBuffer);
  m_fxWaterDarknessMaterial->getUniform("u_portalDepth")->set(m_portalDepthBuffer);
  m_fxWaterDarknessMaterial->getUniform("u_portalPerturb")->set(m_portalPerturbBuffer);

  m_portalFb = gl::FrameBufferBuilder()
                 .texture(::gl::FramebufferAttachment::DepthAttachment, m_portalDepthBuffer)
                 .textureNoBlend(::gl::FramebufferAttachment::ColorAttachment0, m_portalPerturbBuffer)
                 .build();

  m_shadowDepthBuffer->image(ShadowMapSize, ShadowMapSize)
    .set(::gl::TextureMinFilter::Nearest)
    .set(::gl::TextureMagFilter::Nearest)
    .set(::gl::TextureParameterName::TextureWrapS, ::gl::TextureWrapMode::Repeat)
    .set(::gl::TextureParameterName::TextureWrapT, ::gl::TextureWrapMode::Repeat);
  m_shadowDepthFb = gl::FrameBufferBuilder()
                      .textureNoBlend(::gl::FramebufferAttachment::DepthAttachment, m_shadowDepthBuffer)
                      .build();

  // === fxaaFB setup ===
  m_fxaaColorBuffer->set(::gl::TextureParameterName::TextureWrapS, ::gl::TextureWrapMode::ClampToEdge)
    .set(::gl::TextureParameterName::TextureWrapT, ::gl::TextureWrapMode::ClampToEdge)
    .set(::gl::TextureMinFilter::Linear)
    .set(::gl::TextureMagFilter::Linear);
  m_fxDarknessMaterial->getUniform("u_texture")->set(m_fxaaColorBuffer);
  m_fxWaterDarknessMaterial->getUniform("u_texture")->set(m_fxaaColorBuffer);

  m_fxaaFb = gl::FrameBufferBuilder().texture(::gl::FramebufferAttachment::ColorAttachment0, m_fxaaColorBuffer).build();

  // === ssaoFB setup ===
  m_ssaoAOBuffer->set(::gl::TextureParameterName::TextureWrapS, ::gl::TextureWrapMode::ClampToEdge)
    .set(::gl::TextureParameterName::TextureWrapT, ::gl::TextureWrapMode::ClampToEdge)
    .set(::gl::TextureMinFilter::Linear)
    .set(::gl::TextureMagFilter::Linear);
  m_ssaoBlurMaterial->getUniform("u_ao")->set(m_ssaoAOBuffer);

  m_ssaoFb
    = gl::FrameBufferBuilder().textureNoBlend(::gl::FramebufferAttachment::ColorAttachment0, m_ssaoAOBuffer).build();

  // === ssaoBlurFB setup ===
  m_ssaoBlurAOBuffer->set(::gl::TextureParameterName::TextureWrapS, ::gl::TextureWrapMode::ClampToEdge)
    .set(::gl::TextureParameterName::TextureWrapT, ::gl::TextureWrapMode::ClampToEdge)
    .set(::gl::TextureMinFilter::Linear)
    .set(::gl::TextureMagFilter::Linear);
  m_fxDarknessMaterial->getUniform("u_ao")->set(m_ssaoBlurAOBuffer);
  m_fxWaterDarknessMaterial->getUniform("u_ao")->set(m_ssaoBlurAOBuffer);

  m_ssaoBlurFb = gl::FrameBufferBuilder()
                   .textureNoBlend(::gl::FramebufferAttachment::ColorAttachment0, m_ssaoBlurAOBuffer)
                   .build();

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

void RenderPipeline::finalPass(const bool water)
{
  {
    gl::DebugGroup dbg{"ssao-pass"};
    m_ssaoFb->bind();
    scene::RenderContext context{scene::RenderMode::Full};
    scene::Node dummyNode{""};
    context.setCurrentNode(&dummyNode);

    m_fbModel->getMeshes()[0]->setMaterial(m_ssaoMaterial, scene::RenderMode::Full);
    m_fbModel->render(context);

    m_ssaoBlurFb->bind();
    m_fbModel->getMeshes()[0]->setMaterial(m_ssaoBlurMaterial, scene::RenderMode::Full);
    m_fbModel->render(context);
  }

  {
    gl::DebugGroup dbg{"fxaa-pass"};
    m_fxaaFb->bind();
    scene::RenderContext context{scene::RenderMode::Full};
    scene::Node dummyNode{""};
    context.setCurrentNode(&dummyNode);

    m_fbModel->getMeshes()[0]->setMaterial(m_fxaaMaterial, scene::RenderMode::Full);
    m_fbModel->render(context);
  }

  {
    gl::DebugGroup dbg{"postprocess-pass"};
    gl::Framebuffer::unbindAll();
    if(water)
      m_fbModel->getMeshes()[0]->setMaterial(m_fxWaterDarknessMaterial, scene::RenderMode::Full);
    else
      m_fbModel->getMeshes()[0]->setMaterial(m_fxDarknessMaterial, scene::RenderMode::Full);
    scene::RenderContext context{scene::RenderMode::Full};
    scene::Node dummyNode{""};
    context.setCurrentNode(&dummyNode);
    m_fbModel->render(context);
  }
}

void RenderPipeline::update(const scene::Camera& camera, const std::chrono::high_resolution_clock::time_point& time)
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

void RenderPipeline::resize(const scene::Dimension2<size_t>& viewport)
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
  fxaaMesh->setMaterial(m_fxaaMaterial, scene::RenderMode::Full);

  m_fbModel->getMeshes().clear();
  m_fbModel->addMesh(fxaaMesh);
}
} // namespace render