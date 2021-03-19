#include "ssaopass.h"

#include "config.h"
#include "geometrypass.h"
#include "render/scene/shadermanager.h"
#include "util.h"

#include <random>

namespace render::pass
{
SSAOPass::SSAOPass(scene::ShaderManager& shaderManager, const glm::ivec2& viewport, const GeometryPass& geometryPass)
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

  m_material->getUniform("u_normals")->set(geometryPass.getNormalBuffer());
  m_material->getUniform("u_position")->set(geometryPass.getPositionBuffer());

  m_renderMesh->getMaterialGroup().set(scene::RenderMode::Full, m_material);

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
