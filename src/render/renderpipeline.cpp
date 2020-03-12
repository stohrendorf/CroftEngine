#include "renderpipeline.h"

#include "scene/mesh.h"
#include "scene/node.h"
#include "scene/shadermanager.h"

#include <random>

namespace render
{
std::shared_ptr<scene::Model> RenderPipeline::makeFbModel()
{
  auto model = std::make_shared<scene::Model>();
  model->getRenderState().setCullFace(false);
  model->getRenderState().setDepthTest(false);
  model->getRenderState().setDepthWrite(false);
  return model;
}

RenderPipeline::RenderPipeline(scene::ShaderManager& shaderManager, const glm::ivec2& viewport)
    : m_ssaoStage{shaderManager}
    , m_fxaaStage{shaderManager}
    , m_postprocessStage{shaderManager}
{
  resize(viewport);
}

void RenderPipeline::finalPass(const bool water)
{
  m_ssaoStage.render(m_size / 2);
  m_fxaaStage.render(m_size);
  m_postprocessStage.render(water);

  m_fxaaStage.fb->invalidate();
  m_portalStage.fb->invalidate();
  m_geometryStage.fb->invalidate();
  m_ssaoStage.blur.blur2.fb->invalidate();
}

void RenderPipeline::update(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera,
                            const std::chrono::high_resolution_clock::time_point& time)
{
  m_postprocessStage.update(camera, time);
  m_ssaoStage.update(camera);
}

void RenderPipeline::resizeTextures(const glm::ivec2& viewport)
{
  m_geometryStage.resize(viewport);
  m_portalStage.resize(viewport);
  m_ssaoStage.resize(viewport / 2, m_geometryStage);
  m_fxaaStage.resize(viewport, m_geometryStage);
  m_postprocessStage.resize(viewport, m_geometryStage, m_portalStage, m_ssaoStage, m_fxaaStage);
}

void RenderPipeline::buildFramebuffers()
{
  m_portalStage.build();
  m_geometryStage.build();
  m_fxaaStage.build();
  m_ssaoStage.build();
}

RenderPipeline::SSAOStage::SSAOStage(scene::ShaderManager& shaderManager)
    : shader{shaderManager.getSSAO()}
    , material{std::make_shared<scene::Material>(shader)}
    , blur{"ssao", shaderManager}

{
  // generate sample kernel
  std::uniform_real_distribution<float> randomFloats(0, 1);
  std::default_random_engine generator{}; // NOLINT(cert-msc32-c)
  std::vector<glm::vec3> ssaoSamples;
  while(ssaoSamples.size() < 16)
  {
#define SSAO_UNIFORM_VOLUME_SAMPLING
#ifdef SSAO_SAMPLE_CONTRACTION
    glm::vec3 sample{randomFloats(generator) * 2 - 1, randomFloats(generator) * 2 - 1, randomFloats(generator)};
    sample = glm::normalize(sample) * randomFloats(generator);
    // scale samples s.t. they're more aligned to center of kernel
    const float scale = float(i) / 16;
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
  material->getUniform("u_samples[0]")->set(ssaoSamples);

  // generate noise texture
  std::vector<gl::RGB32F> ssaoNoise;
  ssaoNoise.reserve(16);
  for(int i = 0; i < 16; ++i)
  {
    // rotate around z-axis (in tangent space)
    ssaoNoise.emplace_back(randomFloats(generator) * 2 - 1, randomFloats(generator) * 2 - 1, 0.0f);
  }

  noiseTexture = std::make_shared<gl::Texture2D<gl::RGB32F>>(glm::ivec2{4, 4}, "ssao-noise");
  noiseTexture->assign(ssaoNoise.data())
    .set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureMinFilter::Nearest)
    .set(gl::api::TextureMagFilter::Nearest);
  material->getUniform("u_texNoise")->set(noiseTexture);
}

void RenderPipeline::SSAOStage::build()
{
  fb = gl::FrameBufferBuilder()
         .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, aoBuffer)
         .build("ssao-fb");

  blur.build();
}

void RenderPipeline::SSAOStage::resize(const glm::ivec2& viewport, const RenderPipeline::GeometryStage& geometryStage)
{
  aoBuffer = std::make_shared<gl::Texture2D<gl::ScalarByte>>(viewport, "ssao-ao");
  aoBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);

  material->getUniform("u_normals")->set(geometryStage.normalBuffer);
  material->getUniform("u_position")->set(geometryStage.positionBuffer);

  renderModel->getMeshes().clear();
  renderModel->addMesh(
    scene::createQuadFullscreen(gsl::narrow<float>(viewport.x), gsl::narrow<float>(viewport.y), shader->getHandle()));
  renderModel->getMeshes()[0]->getMaterial().set(scene::RenderMode::Full, material);

  blur.resize(viewport, aoBuffer);
}

void RenderPipeline::PortalStage::resize(const glm::ivec2& viewport)
{
  depthBuffer = std::make_shared<gl::TextureDepth<float>>(viewport, "portal-depth");
  depthBuffer->set(gl::api::TextureMinFilter::Linear).set(gl::api::TextureMagFilter::Linear);
  perturbBuffer = std::make_shared<gl::Texture2D<gl::RG32F>>(viewport, "portal-perturb");
  perturbBuffer->set(gl::api::TextureMinFilter::Linear).set(gl::api::TextureMagFilter::Linear);
}

void RenderPipeline::PortalStage::bind(const gl::TextureDepth<float>& depth)
{
  gl::Framebuffer::unbindAll();
  depthBuffer->copyFrom(depth);
  fb->bindWithAttachments();
}

void RenderPipeline::PortalStage::build()
{
  fb = gl::FrameBufferBuilder()
         .texture(gl::api::FramebufferAttachment::DepthAttachment, depthBuffer)
         .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, perturbBuffer)
         .build("portal-fb");
}

void RenderPipeline::GeometryStage::bind(const glm::ivec2& size)
{
  gl::Framebuffer::unbindAll();
  GL_ASSERT(::gl::api::viewport(0, 0, size.x, size.y));
  fb->bindWithAttachments();
}

void RenderPipeline::GeometryStage::build()
{
  fb = gl::FrameBufferBuilder()
         .texture(gl::api::FramebufferAttachment::ColorAttachment0, colorBuffer)
         .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment1, normalBuffer)
         .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment2, positionBuffer)
         .textureNoBlend(gl::api::FramebufferAttachment::DepthAttachment, depthBuffer)
         .build("geometry-fb");
}

void RenderPipeline::GeometryStage::resize(const glm::ivec2& viewport)
{
  depthBuffer = std::make_shared<gl::TextureDepth<float>>(viewport, "geometry-depth");
  depthBuffer->set(gl::api::TextureMinFilter::Linear).set(gl::api::TextureMagFilter::Linear);

  colorBuffer = std::make_shared<gl::Texture2D<gl::SRGBA8>>(viewport, "geometry-color");
  colorBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);

  normalBuffer = std::make_shared<gl::Texture2D<gl::RGB16F>>(viewport, "geometry-normal");
  normalBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Nearest)
    .set(gl::api::TextureMagFilter::Nearest);

  positionBuffer = std::make_shared<gl::Texture2D<gl::RGB32F>>(viewport, "geometry-position");
  positionBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Nearest)
    .set(gl::api::TextureMagFilter::Nearest);
}

void RenderPipeline::FXAAStage::resize(const glm::ivec2& viewport, const GeometryStage& geometryStage)
{
  colorBuffer = std::make_shared<gl::Texture2D<gl::SRGBA8>>(viewport, "fxaa-color");
  colorBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);

  material->getUniform("u_input")->set(geometryStage.colorBuffer);

  model->getMeshes().clear();
  model->addMesh(
    scene::createQuadFullscreen(gsl::narrow<float>(viewport.x), gsl::narrow<float>(viewport.y), shader->getHandle()));
  model->getMeshes()[0]->getMaterial().set(scene::RenderMode::Full, material);
}

RenderPipeline::FXAAStage::FXAAStage(scene::ShaderManager& shaderManager)
    : shader{shaderManager.getFXAA()}
    , material{std::make_shared<scene::Material>(shader)}
{
}

void RenderPipeline::FXAAStage::build()
{
  fb = gl::FrameBufferBuilder().texture(gl::api::FramebufferAttachment::ColorAttachment0, colorBuffer).build("fxaa-fb");
}

void RenderPipeline::FXAAStage::bind()
{
  fb->bindWithAttachments();
}

void RenderPipeline::FXAAStage::render(const glm::ivec2& size)
{
  gl::DebugGroup dbg{"fxaa-pass"};
  GL_ASSERT(gl::api::viewport(0, 0, size.x, size.y));
  bind();

  gl::RenderState state;
  state.setBlend(false);
  state.apply(true);
  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};
  scene::Node dummyNode{""};
  context.setCurrentNode(&dummyNode);

  model->render(context);

  if constexpr(FlushStages)
    GL_ASSERT(gl::api::finish());
}

void RenderPipeline::SSAOStage::update(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera)
{
  material->getUniformBlock("Camera")->bindCameraBuffer(camera);
}

void RenderPipeline::SSAOStage::render(const glm::ivec2& size) const
{
  gl::DebugGroup dbg{"ssao-pass"};
  GL_ASSERT(gl::api::viewport(0, 0, size.x, size.y));
  fb->bindWithAttachments();

  gl::RenderState state;
  state.setBlend(false);
  state.apply(true);
  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};
  scene::Node dummyNode{""};
  context.setCurrentNode(&dummyNode);

  renderModel->render(context);
  blur.render(size);
  fb->invalidate();

  if constexpr(FlushStages)
    GL_ASSERT(gl::api::finish());
}

RenderPipeline::PostprocessStage::PostprocessStage(scene::ShaderManager& shaderManager)
    : darknessShader{shaderManager.getPostprocessing()}
    , darknessMaterial{std::make_shared<scene::Material>(darknessShader)}
    , waterDarknessShader{shaderManager.getPostprocessingWater()}
    , waterDarknessMaterial{std::make_shared<scene::Material>(waterDarknessShader)}

{
  darknessMaterial->getUniform("distortion_power")->set(-1.0f);
  waterDarknessMaterial->getUniform("distortion_power")->set(-2.0f);
}

void RenderPipeline::PostprocessStage::update(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera,
                                              const std::chrono::high_resolution_clock::time_point& time)
{
  const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(time);
  darknessMaterial->getUniform("u_time")->set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
  waterDarknessMaterial->getUniform("u_time")->set(gsl::narrow_cast<float>(now.time_since_epoch().count()));

  darknessMaterial->getUniformBlock("Camera")->bindCameraBuffer(camera);
  waterDarknessMaterial->getUniformBlock("Camera")->bindCameraBuffer(camera);
}

void RenderPipeline::PostprocessStage::resize(const glm::ivec2& viewport,
                                              const RenderPipeline::GeometryStage& geometryStage,
                                              const RenderPipeline::PortalStage& portalStage,
                                              const RenderPipeline::SSAOStage& ssaoStage,
                                              const RenderPipeline::FXAAStage& fxaaStage)
{
  darknessMaterial->getUniform("u_portalDepth")->set(portalStage.depthBuffer);
  darknessMaterial->getUniform("u_portalPerturb")->set(portalStage.perturbBuffer);
  darknessMaterial->getUniform("u_depth")->set(geometryStage.depthBuffer);
  darknessMaterial->getUniform("u_ao")->set(ssaoStage.blur.blur2.buffer);
  darknessMaterial->getUniform("u_texture")->set(fxaaStage.colorBuffer);

  waterDarknessMaterial->getUniform("u_portalDepth")->set(portalStage.depthBuffer);
  waterDarknessMaterial->getUniform("u_portalPerturb")->set(portalStage.perturbBuffer);
  waterDarknessMaterial->getUniform("u_depth")->set(geometryStage.depthBuffer);
  waterDarknessMaterial->getUniform("u_ao")->set(ssaoStage.blur.blur2.buffer);
  waterDarknessMaterial->getUniform("u_texture")->set(fxaaStage.colorBuffer);

  model->getMeshes().clear();
  model->addMesh(scene::createQuadFullscreen(
    gsl::narrow<float>(viewport.x), gsl::narrow<float>(viewport.y), darknessShader->getHandle()));
  model->getMeshes()[0]->getMaterial().set(scene::RenderMode::Full, darknessMaterial);
  waterModel->getMeshes().clear();
  waterModel->addMesh(scene::createQuadFullscreen(
    gsl::narrow<float>(viewport.x), gsl::narrow<float>(viewport.y), waterDarknessShader->getHandle()));
  waterModel->getMeshes()[0]->getMaterial().set(scene::RenderMode::Full, waterDarknessMaterial);
}

void RenderPipeline::PostprocessStage::render(bool water)
{
  gl::DebugGroup dbg{"postprocess-pass"};
  gl::Framebuffer::unbindAll();

  gl::RenderState state;
  state.setBlend(false);
  state.apply(true);
  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};
  scene::Node dummyNode{""};
  context.setCurrentNode(&dummyNode);
  if(water)
    waterModel->render(context);
  else
    model->render(context);

  if constexpr(FlushStages)
    GL_ASSERT(gl::api::finish());
}
} // namespace render