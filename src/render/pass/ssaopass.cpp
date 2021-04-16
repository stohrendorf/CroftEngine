#include "ssaopass.h"

#include "config.h"
#include "geometrypass.h"
#include "render/scene/materialmanager.h"

#include <random>

namespace render::pass
{
SSAOPass::SSAOPass(scene::MaterialManager& materialManager,
                   const glm::ivec2& viewport,
                   const GeometryPass& geometryPass)
    : m_material{materialManager.getSSAO()}
    , m_renderMesh{scene::createScreenQuad(m_material, "ssao")}
    , m_aoBuffer{std::make_shared<gl::Texture2D<gl::Scalar16F>>(viewport, "ssao-ao")}
    , m_blur{"ssao", materialManager, 4, false}
{
  // generate sample kernel
  std::uniform_real_distribution<float> randomFloats(0, 1);
  // NOLINTNEXTLINE(cert-msc32-c, cert-msc51-cpp)
  std::default_random_engine generator{};
  std::vector<glm::vec3> ssaoSamples;
  while(ssaoSamples.size() < 64)
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
  m_renderMesh->bind("u_samples[0]",
                     [ssaoSamples](const render::scene::Node& /*node*/,
                                   const render::scene::Mesh& /*mesh*/,
                                   gl::Uniform& uniform) { uniform.set(ssaoSamples); });
  m_aoBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);

  m_renderMesh->bind("u_normals",
                     [buffer = geometryPass.getNormalBuffer()](const render::scene::Node& /*node*/,
                                                               const render::scene::Mesh& /*mesh*/,
                                                               gl::Uniform& uniform) { uniform.set(buffer); });
  m_renderMesh->bind("u_position",
                     [buffer = geometryPass.getPositionBuffer()](const render::scene::Node& /*node*/,
                                                                 const render::scene::Mesh& /*mesh*/,
                                                                 gl::Uniform& uniform) { uniform.set(buffer); });

  m_blur.setInput(m_aoBuffer);

  m_fb = gl::FrameBufferBuilder()
           .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_aoBuffer)
           .build("ssao-fb");
}

void SSAOPass::updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera)
{
  m_material->getUniformBlock("Camera")->bindCameraBuffer(camera);
}

void SSAOPass::render(const glm::ivec2& size)
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

  if constexpr(FlushPasses)
    GL_ASSERT(gl::api::finish());
}
} // namespace render::pass
