#include "renderpipeline.h"

#include "scene/materialmanager.h"
#include "scene/mesh.h"
#include "scene/node.h"
#include "scene/shadermanager.h"

#include <gl/texturedepth.h>
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
{
  resize(materialManager, viewport, true);
}

void RenderPipeline::compositionPass(const bool water)
{
  if(m_renderSettings.waterDenoise)
  {
    BOOST_ASSERT(m_portalStage != nullptr);
    m_portalStage->renderBlur();
  }
  BOOST_ASSERT(m_ssaoStage != nullptr);
  m_ssaoStage->render(m_size / 2);
  BOOST_ASSERT(m_fxaaStage != nullptr);
  m_fxaaStage->render(m_size);
  BOOST_ASSERT(m_compositionStage != nullptr);
  m_compositionStage->render(water, m_renderSettings);
}

void RenderPipeline::updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera)
{
  BOOST_ASSERT(m_compositionStage != nullptr);
  m_compositionStage->updateCamera(camera);
  BOOST_ASSERT(m_ssaoStage != nullptr);
  m_ssaoStage->updateCamera(camera);
}

void RenderPipeline::apply(const RenderSettings& renderSettings, scene::MaterialManager& materialManager)
{
  m_renderSettings = renderSettings;
  resize(materialManager, m_size, true);
}

void RenderPipeline::resize(scene::MaterialManager& materialManager, const glm::ivec2& viewport, bool force)
{
  if(!force && m_size == viewport)
  {
    return;
  }

  m_size = viewport;

  m_geometryStage = std::make_shared<GeometryStage>(viewport);
  m_portalStage = std::make_shared<PortalStage>(*materialManager.getShaderManager(), viewport);
  m_ssaoStage = std::make_shared<SSAOStage>(*materialManager.getShaderManager(), viewport / 2, *m_geometryStage);
  m_fxaaStage = std::make_shared<FXAAStage>(*materialManager.getShaderManager(), viewport, *m_geometryStage);
  m_compositionStage = std::make_shared<CompositionStage>(
    materialManager, m_renderSettings, viewport, *m_geometryStage, *m_portalStage, *m_ssaoStage, *m_fxaaStage);
}

RenderPipeline::SSAOStage::SSAOStage(scene::ShaderManager& shaderManager,
                                     const glm::ivec2& viewport,
                                     const GeometryStage& geometryStage)
    : m_shader{shaderManager.getSSAO()}
    , m_material{std::make_shared<scene::Material>(m_shader)}
    , m_renderMesh{createFbMesh(viewport, m_shader->getHandle())}
    , m_noiseTexture{std::make_shared<gl::Texture2D<gl::RGB32F>>(glm::ivec2{4, 4}, "ssao-noise")}
    , m_aoBuffer{std::make_shared<gl::Texture2D<gl::Scalar16F>>(viewport, "ssao-ao")}
    , m_blur{"ssao", shaderManager, 4, true, false}
{
  // generate sample kernel
  std::uniform_real_distribution<float> randomFloats(0, 1);
  // NOLINTNEXTLINE(cert-msc32-c, cert-msc51-cpp)
  std::default_random_engine generator{};
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
  m_material->getUniform("u_samples[0]")->set(ssaoSamples);

  // generate noise texture
  std::vector<gl::RGB32F> ssaoNoise;
  ssaoNoise.reserve(16);
  for(int i = 0; i < 16; ++i)
  {
    // rotate around z-axis (in tangent space)
    ssaoNoise.emplace_back(randomFloats(generator) * 2 - 1, randomFloats(generator) * 2 - 1, 0.0f);
  }

  m_noiseTexture->assign(ssaoNoise.data())
    .set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureMinFilter::Nearest)
    .set(gl::api::TextureMagFilter::Nearest);
  m_material->getUniform("u_texNoise")->set(m_noiseTexture);

  m_aoBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);

  m_material->getUniform("u_normals")->set(geometryStage.getNormalBuffer());
  m_material->getUniform("u_position")->set(geometryStage.getPositionBuffer());

  m_renderMesh->getMaterial().set(scene::RenderMode::Full, m_material);

  m_blur.setInput(m_aoBuffer);

  m_fb = gl::FrameBufferBuilder()
           .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_aoBuffer)
           .build("ssao-fb");
}

RenderPipeline::PortalStage::PortalStage(scene::ShaderManager& shaderManager, const glm::vec2& viewport)
    : m_depthBuffer{std::make_shared<gl::TextureDepth<float>>(viewport, "portal-depth")}
    , m_perturbBuffer{std::make_shared<gl::Texture2D<gl::RG32F>>(viewport, "portal-perturb")}
    , m_blur{"perturb", shaderManager, 4, true, false}
{
  m_depthBuffer->set(gl::api::TextureMinFilter::Linear).set(gl::api::TextureMagFilter::Linear);
  m_perturbBuffer->set(gl::api::TextureMinFilter::Linear).set(gl::api::TextureMagFilter::Linear);
  m_blur.setInput(m_perturbBuffer);

  m_fb = gl::FrameBufferBuilder()
           .texture(gl::api::FramebufferAttachment::DepthAttachment, m_depthBuffer)
           .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_perturbBuffer)
           .build("portal-fb");
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void RenderPipeline::PortalStage::bind(const gl::TextureDepth<float>& depth)
{
  gl::Framebuffer::unbindAll();
  m_depthBuffer->copyFrom(depth);
  m_fb->bindWithAttachments();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void RenderPipeline::GeometryStage::bind(const glm::ivec2& size)
{
  gl::Framebuffer::unbindAll();
  GL_ASSERT(::gl::api::viewport(0, 0, size.x, size.y));
  m_fb->bindWithAttachments();
}

RenderPipeline::GeometryStage::GeometryStage(const glm::ivec2& viewport)
    : m_depthBuffer{std::make_shared<gl::TextureDepth<float>>(viewport, "geometry-depth")}
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGBA8>>(viewport, "geometry-color")}
    , m_positionBuffer{std::make_shared<gl::Texture2D<gl::RGB32F>>(viewport, "geometry-position")}
    , m_normalBuffer{std::make_shared<gl::Texture2D<gl::RGB16F>>(viewport, "geometry-normal")}
{
  m_depthBuffer->set(gl::api::TextureMinFilter::Linear).set(gl::api::TextureMagFilter::Linear);
  m_colorBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);
  m_positionBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Nearest)
    .set(gl::api::TextureMagFilter::Nearest);
  m_normalBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Nearest)
    .set(gl::api::TextureMagFilter::Nearest);

  m_fb = gl::FrameBufferBuilder()
           .texture(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer)
           .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment1, m_normalBuffer)
           .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment2, m_positionBuffer)
           .textureNoBlend(gl::api::FramebufferAttachment::DepthAttachment, m_depthBuffer)
           .build("geometry-fb");
}

RenderPipeline::FXAAStage::FXAAStage(scene::ShaderManager& shaderManager,
                                     const glm::ivec2& viewport,
                                     const GeometryStage& geometryStage)
    : m_shader{shaderManager.getFXAA()}
    , m_material{std::make_shared<scene::Material>(m_shader)}
    , m_mesh{createFbMesh(viewport, m_shader->getHandle())}
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGBA8>>(viewport, "fxaa-color")}
{
  m_colorBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);

  m_material->getUniform("u_input")->set(geometryStage.getColorBuffer());

  m_mesh->getMaterial().set(scene::RenderMode::Full, m_material);

  m_fb = gl::FrameBufferBuilder()
           .texture(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer)
           .build("fxaa-fb");
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void RenderPipeline::FXAAStage::bind()
{
  m_fb->bindWithAttachments();
}

void RenderPipeline::FXAAStage::render(const glm::ivec2& size)
{
  SOGLB_DEBUGGROUP("fxaa-pass");
  GL_ASSERT(gl::api::viewport(0, 0, size.x, size.y));
  bind();

  gl::RenderState state;
  state.setBlend(false);
  state.apply(true);
  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};

  m_mesh->render(context);

  if constexpr(FlushStages)
    GL_ASSERT(gl::api::finish());
}

void RenderPipeline::SSAOStage::updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera)
{
  m_material->getUniformBlock("Camera")->bindCameraBuffer(camera);
}

void RenderPipeline::SSAOStage::render(const glm::ivec2& size)
{
  SOGLB_DEBUGGROUP("ssao-pass");
  GL_ASSERT(gl::api::viewport(0, 0, size.x, size.y));
  m_fb->bindWithAttachments();

  gl::RenderState state;
  state.setBlend(false);
  state.apply(true);
  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};

  m_renderMesh->render(context);
  m_blur.render();

  if constexpr(FlushStages)
    GL_ASSERT(gl::api::finish());
}

RenderPipeline::CompositionStage::CompositionStage(scene::MaterialManager& materialManager,
                                                   const RenderSettings& renderSettings,
                                                   const glm::ivec2& viewport,
                                                   const GeometryStage& geometryStage,
                                                   const PortalStage& portalStage,
                                                   const SSAOStage& ssaoStage,
                                                   const FXAAStage& fxaaStage)
    : m_compositionMaterial{materialManager.getComposition(
      false, renderSettings.lensDistortion, renderSettings.dof, renderSettings.filmGrain)}
    , m_waterCompositionMaterial{materialManager.getComposition(
        true, renderSettings.lensDistortion, renderSettings.dof, renderSettings.filmGrain)}
    , m_crtMaterial{materialManager.getCrt()}
    , m_mesh{createFbMesh(viewport, m_compositionMaterial->getShaderProgram()->getHandle())}
    , m_waterMesh{createFbMesh(viewport, m_waterCompositionMaterial->getShaderProgram()->getHandle())}
    , m_crtMesh{createFbMesh(viewport, m_crtMaterial->getShaderProgram()->getHandle())}
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGBA8>>(viewport, "composition-color")}
{
  const glm::ivec2 resolution{256, 256};

  std::uniform_real_distribution<float> randomFloats(0, 1);
  std::default_random_engine generator{}; // NOLINT(cert-msc32-c, cert-msc51-cpp)

  std::vector<gl::ScalarByte> noiseData;
  noiseData.resize(resolution.x * resolution.y);
  for(auto& i : noiseData)
  {
    const auto value = randomFloats(generator);
    i = gl::ScalarByte{static_cast<uint8_t>(value * value * 255)};
  }
  m_noise = std::make_shared<gl::Texture2D<gl::ScalarByte>>(resolution, "composition-noise");
  m_noise->assign(noiseData.data())
    .set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);

  m_compositionMaterial->getUniform("u_portalDepth")->set(portalStage.getDepthBuffer());
  if(renderSettings.waterDenoise)
    m_compositionMaterial->getUniform("u_portalPerturb")->set(portalStage.getBlurredTexture());
  else
    m_compositionMaterial->getUniform("u_portalPerturb")->set(portalStage.getNoisyTexture());
  m_compositionMaterial->getUniform("u_depth")->set(geometryStage.getDepthBuffer());
  m_compositionMaterial->getUniform("u_ao")->set(ssaoStage.getBlurredTexture());
  m_compositionMaterial->getUniform("u_texture")->set(fxaaStage.getColorBuffer());

  m_waterCompositionMaterial->getUniform("u_portalDepth")->set(portalStage.getDepthBuffer());
  if(renderSettings.waterDenoise)
    m_waterCompositionMaterial->getUniform("u_portalPerturb")->set(portalStage.getBlurredTexture());
  else
    m_waterCompositionMaterial->getUniform("u_portalPerturb")->set(portalStage.getNoisyTexture());
  m_waterCompositionMaterial->getUniform("u_depth")->set(geometryStage.getDepthBuffer());
  m_waterCompositionMaterial->getUniform("u_ao")->set(ssaoStage.getBlurredTexture());
  m_waterCompositionMaterial->getUniform("u_texture")->set(fxaaStage.getColorBuffer());

  m_colorBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);

  m_crtMaterial->getUniform("u_input")->set(m_colorBuffer);
  m_crtMaterial->getUniform("u_noise")->set(m_noise);

  m_fb = gl::FrameBufferBuilder()
           .texture(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer)
           .build("composition-fb");

  m_mesh->getMaterial().set(scene::RenderMode::Full, m_compositionMaterial);
  m_waterMesh->getMaterial().set(scene::RenderMode::Full, m_waterCompositionMaterial);
  m_crtMesh->getMaterial().set(scene::RenderMode::Full, m_crtMaterial);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void RenderPipeline::CompositionStage::updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera)
{
  m_compositionMaterial->getUniformBlock("Camera")->bindCameraBuffer(camera);
  m_waterCompositionMaterial->getUniformBlock("Camera")->bindCameraBuffer(camera);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void RenderPipeline::CompositionStage::render(bool water, const RenderSettings& renderSettings)
{
  SOGLB_DEBUGGROUP("postprocess-pass");
  if(renderSettings.crt)
    m_fb->bindWithAttachments();
  else
    gl::Framebuffer::unbindAll();

  gl::RenderState state;
  state.setBlend(false);
  state.apply(true);
  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};
  if(water)
    m_waterMesh->render(context);
  else
    m_mesh->render(context);

  if(renderSettings.crt)
  {
    gl::Framebuffer::unbindAll();
    m_crtMesh->render(context);
  }

  if constexpr(FlushStages)
    GL_ASSERT(gl::api::finish());
}
} // namespace render
