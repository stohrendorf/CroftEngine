#include "renderpipeline.h"

#include "scene/materialmanager.h"
#include "scene/mesh.h"
#include "scene/node.h"
#include "scene/shadermanager.h"

#include <random>

namespace render
{
std::shared_ptr<scene::Mesh> RenderPipeline::createFbMesh(const glm::ivec2& size, const gl::Program& program)
{
  auto mesh = scene::createQuadFullscreen(gsl::narrow<float>(size.x), gsl::narrow<float>(size.y), program);
  mesh->getRenderState().setCullFace(false);
  mesh->getRenderState().setDepthTest(false);
  mesh->getRenderState().setDepthWrite(false);
  return mesh;
}

RenderPipeline::RenderPipeline(scene::MaterialManager& materialManager, const glm::ivec2& viewport)
    : m_ssaoStage{*materialManager.getShaderManager()}
    , m_fxaaStage{*materialManager.getShaderManager()}
    , m_compositionStage{materialManager}
{
  resize(viewport);
}

void RenderPipeline::compositionPass(const bool water, const bool crt)
{
  m_ssaoStage.render(m_size / 2);
  m_fxaaStage.render(m_size);
  m_compositionStage.render(water, crt);

  m_fxaaStage.fb->invalidate();
  m_portalStage.fb->invalidate();
  m_geometryStage.fb->invalidate();
  m_ssaoStage.blur.invalidate();
}

void RenderPipeline::update(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera)
{
  m_compositionStage.update(camera);
  m_ssaoStage.update(camera);
}

void RenderPipeline::resizeTextures(const glm::ivec2& viewport)
{
  m_geometryStage.resize(viewport);
  m_portalStage.resize(viewport);
  m_ssaoStage.resize(viewport / 2, m_geometryStage);
  m_fxaaStage.resize(viewport, m_geometryStage);
  m_compositionStage.resize(viewport, m_geometryStage, m_portalStage, m_ssaoStage, m_fxaaStage);
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

void RenderPipeline::SSAOStage::resize(const glm::ivec2& viewport, const RenderPipeline::GeometryStage& geometryStage)
{
  aoBuffer = std::make_shared<gl::Texture2D<gl::ScalarByte>>(viewport, "ssao-ao");
  aoBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);

  material->getUniform("u_normals")->set(geometryStage.normalBuffer);
  material->getUniform("u_position")->set(geometryStage.positionBuffer);

  renderMesh = createFbMesh(viewport, shader->getHandle());
  renderMesh->getMaterial().set(scene::RenderMode::Full, material);

  blur.resize(viewport, aoBuffer);

  fb = gl::FrameBufferBuilder()
         .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, aoBuffer)
         .build("ssao-fb");
}

void RenderPipeline::PortalStage::resize(const glm::ivec2& viewport)
{
  depthBuffer = std::make_shared<gl::TextureDepth<float>>(viewport, "portal-depth");
  depthBuffer->set(gl::api::TextureMinFilter::Linear).set(gl::api::TextureMagFilter::Linear);
  perturbBuffer = std::make_shared<gl::Texture2D<gl::RG32F>>(viewport, "portal-perturb");
  perturbBuffer->set(gl::api::TextureMinFilter::Linear).set(gl::api::TextureMagFilter::Linear);

  fb = gl::FrameBufferBuilder()
         .texture(gl::api::FramebufferAttachment::DepthAttachment, depthBuffer)
         .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, perturbBuffer)
         .build("portal-fb");
}

void RenderPipeline::PortalStage::bind(const gl::TextureDepth<float>& depth)
{
  gl::Framebuffer::unbindAll();
  depthBuffer->copyFrom(depth);
  fb->bindWithAttachments();
}

void RenderPipeline::GeometryStage::bind(const glm::ivec2& size)
{
  gl::Framebuffer::unbindAll();
  GL_ASSERT(::gl::api::viewport(0, 0, size.x, size.y));
  fb->bindWithAttachments();
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

  fb = gl::FrameBufferBuilder()
         .texture(gl::api::FramebufferAttachment::ColorAttachment0, colorBuffer)
         .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment1, normalBuffer)
         .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment2, positionBuffer)
         .textureNoBlend(gl::api::FramebufferAttachment::DepthAttachment, depthBuffer)
         .build("geometry-fb");
}

void RenderPipeline::FXAAStage::resize(const glm::ivec2& viewport, const GeometryStage& geometryStage)
{
  colorBuffer = std::make_shared<gl::Texture2D<gl::SRGBA8>>(viewport, "fxaa-color");
  colorBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);

  material->getUniform("u_input")->set(geometryStage.colorBuffer);

  mesh = createFbMesh(viewport, shader->getHandle());
  mesh->getMaterial().set(scene::RenderMode::Full, material);

  fb = gl::FrameBufferBuilder().texture(gl::api::FramebufferAttachment::ColorAttachment0, colorBuffer).build("fxaa-fb");
}

RenderPipeline::FXAAStage::FXAAStage(scene::ShaderManager& shaderManager)
    : shader{shaderManager.getFXAA()}
    , material{std::make_shared<scene::Material>(shader)}
{
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

  mesh->render(context);

  if constexpr(FlushStages)
    GL_ASSERT(gl::api::finish());
}

void RenderPipeline::SSAOStage::update(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera)
{
  material->getUniformBlock("Camera")->bindCameraBuffer(camera);
}

void RenderPipeline::SSAOStage::render(const glm::ivec2& size)
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

  renderMesh->render(context);
  blur.render(size);
  fb->invalidate();

  if constexpr(FlushStages)
    GL_ASSERT(gl::api::finish());
}

RenderPipeline::CompositionStage::CompositionStage(scene::MaterialManager& materialManager)
    : compositionMaterial{materialManager.getComposition(false)}
    , waterCompositionMaterial{materialManager.getComposition(true)}
    , crtMaterial{materialManager.getCrt()}

{
  const glm::ivec2 resolution{256, 256};

  std::uniform_real_distribution<float> randomFloats(0, 1);
  std::default_random_engine generator{}; // NOLINT(cert-msc32-c)

  std::vector<gl::ScalarByte> noiseData;
  noiseData.resize(resolution.x * resolution.y);
  for(auto& i : noiseData)
  {
    const auto value = randomFloats(generator);
    i = gl::ScalarByte{static_cast<uint8_t>(value * value * 255)};
  }
  noise = std::make_shared<gl::Texture2D<gl::ScalarByte>>(resolution, "composition-noise");
  noise->assign(noiseData.data())
    .set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);
}

void RenderPipeline::CompositionStage::update(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera)
{
  compositionMaterial->getUniformBlock("Camera")->bindCameraBuffer(camera);
  waterCompositionMaterial->getUniformBlock("Camera")->bindCameraBuffer(camera);
}

void RenderPipeline::CompositionStage::resize(const glm::ivec2& viewport,
                                              const RenderPipeline::GeometryStage& geometryStage,
                                              const RenderPipeline::PortalStage& portalStage,
                                              const RenderPipeline::SSAOStage& ssaoStage,
                                              const RenderPipeline::FXAAStage& fxaaStage)
{
  compositionMaterial->getUniform("u_portalDepth")->set(portalStage.depthBuffer);
  compositionMaterial->getUniform("u_portalPerturb")->set(portalStage.perturbBuffer);
  compositionMaterial->getUniform("u_depth")->set(geometryStage.depthBuffer);
  compositionMaterial->getUniform("u_ao")->set(ssaoStage.blur.getBlurredTexture());
  compositionMaterial->getUniform("u_texture")->set(fxaaStage.colorBuffer);

  waterCompositionMaterial->getUniform("u_portalDepth")->set(portalStage.depthBuffer);
  waterCompositionMaterial->getUniform("u_portalPerturb")->set(portalStage.perturbBuffer);
  waterCompositionMaterial->getUniform("u_depth")->set(geometryStage.depthBuffer);
  waterCompositionMaterial->getUniform("u_ao")->set(ssaoStage.blur.getBlurredTexture());
  waterCompositionMaterial->getUniform("u_texture")->set(fxaaStage.colorBuffer);

  colorBuffer = std::make_shared<gl::Texture2D<gl::SRGBA8>>(viewport, "composition-color");
  colorBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);

  crtMaterial->getUniform("u_input")->set(colorBuffer);
  crtMaterial->getUniform("u_noise")->set(noise);

  fb = gl::FrameBufferBuilder()
         .texture(gl::api::FramebufferAttachment::ColorAttachment0, colorBuffer)
         .build("composition-fb");

  mesh = createFbMesh(viewport, compositionMaterial->getShaderProgram()->getHandle());
  mesh->getMaterial().set(scene::RenderMode::Full, compositionMaterial);
  waterMesh = createFbMesh(viewport, waterCompositionMaterial->getShaderProgram()->getHandle());
  waterMesh->getMaterial().set(scene::RenderMode::Full, waterCompositionMaterial);
  crtMesh = createFbMesh(viewport, crtMaterial->getShaderProgram()->getHandle());
  crtMesh->getMaterial().set(scene::RenderMode::Full, crtMaterial);
}

void RenderPipeline::CompositionStage::render(bool water, bool crt)
{
  gl::DebugGroup dbg{"postprocess-pass"};
  if(crt)
    fb->bindWithAttachments();
  else
    gl::Framebuffer::unbindAll();

  gl::RenderState state;
  state.setBlend(false);
  state.apply(true);
  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};
  scene::Node dummyNode{""};
  context.setCurrentNode(&dummyNode);
  if(water)
    waterMesh->render(context);
  else
    mesh->render(context);

  if(crt)
  {
    gl::Framebuffer::unbindAll();
    crtMesh->render(context);
  }

  if constexpr(FlushStages)
    GL_ASSERT(gl::api::finish());
}
} // namespace render
