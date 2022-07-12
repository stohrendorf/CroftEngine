#include "csm.h"

#include "blur.h"
#include "camera.h"
#include "materialgroup.h"
#include "materialmanager.h"
#include "mesh.h"
#include "rendercontext.h"
#include "rendermode.h"

#include <algorithm>
#include <gl/buffer.h>
#include <gl/debuggroup.h>
#include <gl/framebuffer.h>
#include <gl/glassert.h>
#include <gl/pixel.h>
#include <gl/program.h>
#include <gl/renderstate.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texturedepth.h>
#include <gl/texturehandle.h>
#include <glm/common.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <gslu.h>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace render::scene
{
class Node;
}

namespace render::scene
{
void CSM::Split::init(int32_t resolution, size_t idx, MaterialManager& materialManager)
{
  auto sampler = gsl::make_unique<gl::Sampler>("csm-texture/" + std::to_string(idx) + "-sampler")
                 | set(gl::api::TextureMinFilter::Nearest) | set(gl::api::TextureMagFilter::Nearest)
                 | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToBorder)
                 | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToBorder)
                 | set(gl::api::TextureCompareMode::None);
  sampler->setBorderColor(glm::vec4{1.0f});
  depthTextureHandle = std::make_shared<gl::TextureHandle<gl::TextureDepth<float>>>(
    gsl::make_shared<gl::TextureDepth<float>>(glm::ivec2{resolution, resolution}, "csm-texture/" + std::to_string(idx)),
    std::move(sampler));

  depthFramebuffer
    = gl::FrameBufferBuilder()
        .textureNoBlend(gl::api::FramebufferAttachment::DepthAttachment, depthTextureHandle->getTexture())
        .build("csm-split-fb/" + std::to_string(idx));

  squaredTextureHandle = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::RG16F>>>(
    gsl::make_shared<gl::Texture2D<gl::RG16F>>(glm::ivec2{resolution, resolution},
                                               "csm-texture/" + std::to_string(idx) + "/squared"),
    gsl::make_unique<gl::Sampler>("csm-texture/" + std::to_string(idx) + "/squared-sampler")
      | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear)
      | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
      | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge));
  squareFramebuffer
    = gl::FrameBufferBuilder()
        .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, squaredTextureHandle->getTexture())
        .build("csm-split-fb/" + std::to_string(idx) + "/square");

  squareMaterial = materialManager.getVSMSquare();

  squareMesh = createScreenQuad(squareMaterial, "csm/" + std::to_string(idx));
  squareMesh->bind("u_shadow",
                   [this](const Node* /*node*/, const Mesh& /*mesh*/, gl::Uniform& uniform)
                   {
                     uniform.set(gsl::not_null{depthTextureHandle});
                   });
  squareMesh->getRenderState().merge(squareFramebuffer->getRenderState());
  squareMesh->getMaterialGroup().set(RenderMode::Full, squareMaterial);

  squareBlur = std::make_shared<SeparableBlur<gl::RG16F>>(
    "squareBlur-" + std::to_string(idx), materialManager, uint8_t{2}, true);
  squareBlur->setInput(gsl::not_null{squaredTextureHandle});
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void CSM::Split::renderSquare()
{
  SOGLB_DEBUGGROUP("vsm-square-pass");
  squareFramebuffer->bind();

  RenderContext context{RenderMode::Full, std::nullopt};
  squareMesh->render(nullptr, context);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void CSM::Split::renderBlur()
{
  SOGLB_DEBUGGROUP("vsm-blur-pass");
  squareBlur->render();
}

CSM::CSM(int32_t resolution, MaterialManager& materialManager)
    : m_resolution{resolution}
{
  static_assert(CSMBuffer::NSplits > 0);
  Expects(resolution > 0);

  for(size_t i = 0; i < CSMBuffer::NSplits; ++i)
  {
    m_splits[i].init(resolution, i, materialManager);
  }
}

namespace
{
struct SplitGetter
{
  template<size_t... Is>
  static std::array<gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::RG16F>>>, CSMBuffer::NSplits>
    getBlurred(const std::array<CSM::Split, CSMBuffer::NSplits>& splits, std::index_sequence<Is...>)
  {
    return {{splits[Is].squareBlur->getBlurredTexture()...}};
  }

  static std::array<gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::RG16F>>>, CSMBuffer::NSplits>
    getBlurred(const std::array<CSM::Split, CSMBuffer::NSplits>& splits)
  {
    return getBlurred(splits, std::make_index_sequence<CSMBuffer::NSplits>());
  }

  template<size_t... Is>
  static std::array<gslu::nn_shared<gl::TextureDepth<float>>, CSMBuffer::NSplits>
    getDepth(const std::array<CSM::Split, CSMBuffer::NSplits>& splits, std::index_sequence<Is...>)
  {
    return {{splits[Is].depthTextureHandle->getTexture()...}};
  }

  static std::array<gslu::nn_shared<gl::TextureDepth<float>>, CSMBuffer::NSplits>
    getDepth(const std::array<CSM::Split, CSMBuffer::NSplits>& splits)
  {
    return getDepth(splits, std::make_index_sequence<CSMBuffer::NSplits>());
  }
};
} // namespace

std::array<gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::RG16F>>>, CSMBuffer::NSplits> CSM::getTextures() const
{
  return SplitGetter::getBlurred(m_splits);
}

std::array<gslu::nn_shared<gl::TextureDepth<float>>, CSMBuffer::NSplits> CSM::getDepthTextures() const
{
  return SplitGetter::getDepth(m_splits);
}

std::array<glm::mat4, CSMBuffer::NSplits> CSM::getMatrices(const glm::mat4& modelMatrix) const
{
  std::array<glm::mat4, CSMBuffer::NSplits> result{};
  std::transform(m_splits.begin(),
                 m_splits.end(),
                 result.begin(),
                 [modelMatrix](const Split& split)
                 {
                   return split.vpMatrix * modelMatrix;
                 });
  return result;
}

void CSM::updateCamera(const Camera& camera)
{
  //Start off by calculating the split distances
  const float nearClip = camera.getNearPlane();
  const float farClip = camera.getFarPlane();

  std::vector<float> cascadeSplits;
  cascadeSplits.emplace_back(nearClip);
#if 0
  for(size_t i = 0; i < m_splits.size(); ++i)
  {
    static constexpr float Lambda = 0.9f;
    const auto ir = static_cast<float>(i + 1) / static_cast<float>(m_splits.size());
    const float zi
      = Lambda * nearClip * std::pow(farClip / nearClip, ir) + (1.0f - Lambda) * (nearClip + ir * (farClip - nearClip));
    cascadeSplits.emplace_back(zi);
    m_splits[i].end = -zi;
  }
#else
  for(size_t i = 0; i < m_splits.size(); ++i)
  {
    const auto ir = static_cast<float>(i + 1) / static_cast<float>(m_splits.size());
    const auto zi = nearClip + ir * (farClip - nearClip);
    cascadeSplits.emplace_back(zi);
  }
#endif

  const auto position = camera.getPosition();
  const auto up = camera.getUpVector();
  const auto right = camera.getRightVector();
  const auto forward = camera.getFrontVector();

  for(size_t cascadeIterator = 0; cascadeIterator < m_splits.size(); ++cascadeIterator)
  {
    const auto nc = position + forward * cascadeSplits[cascadeIterator];
    const auto fc = position + forward * cascadeSplits[cascadeIterator + 1];

    const auto Hnear = glm::tan(camera.getFieldOfViewY() / 2) * cascadeSplits[cascadeIterator];
    const auto Wnear = Hnear * camera.getAspectRatio();
    const auto Hfar = glm::tan(camera.getFieldOfViewY() / 2) * cascadeSplits[cascadeIterator + 1];
    const auto Wfar = Hfar * camera.getAspectRatio();

    // frustum corners are in world space
    const std::array<glm::vec3, 8> frustumCorners{
      fc + up * Hfar - right * Wfar,
      fc + up * Hfar + right * Wfar,
      fc - up * Hfar - right * Wfar,
      fc - up * Hfar + right * Wfar,
      nc + up * Hnear - right * Wnear,
      nc + up * Hnear + right * Wnear,
      nc - up * Hnear - right * Wnear,
      nc - up * Hnear + right * Wnear,
    };

    // calculate frustum bbox as seen from the light
    const auto lightViewWS = glm::lookAt(glm::vec3{0.0f}, m_lightDir, m_lightDirOrtho);

    glm::vec3 bboxMin{std::numeric_limits<float>::max()};
    glm::vec3 bboxMax{std::numeric_limits<float>::lowest()};
    for(const auto& corner : frustumCorners)
    {
      const auto cornerLS = glm::vec3{lightViewWS * glm::vec4{corner, 1.0f}};
      bboxMin = glm::min(bboxMin, cornerLS);
      bboxMax = glm::max(bboxMax, cornerLS);
    }

    // extend the bboxes and snap to a grid to avoid shadow jumping
    static constexpr float SnapSize = 512.0f;
    bboxMin = glm::floor(bboxMin / SnapSize) * SnapSize;
    bboxMax = glm::ceil(bboxMax / SnapSize) * SnapSize;

    m_splits[cascadeIterator].vpMatrix
      = glm::ortho(bboxMin.x, bboxMax.x, bboxMin.y, bboxMax.y, -bboxMax.z, -bboxMin.z) * lightViewWS;
  }
}

void CSM::renderSquare()
{
  GL_ASSERT(gl::api::memoryBarrier(gl::api::MemoryBarrierMask::AllBarrierBits));

  m_splits.at(m_activeSplit).renderSquare();
};

gl::UniformBuffer<CSMBuffer>& CSM::getBuffer(const glm::mat4& modelMatrix)
{
  m_bufferData.lightMVP = getMatrices(modelMatrix);
  m_bufferData.lightDir = glm::vec4{m_lightDir, 0.0f};
  m_buffer.setData(m_bufferData, gl::api::BufferUsage::DynamicDraw);
  return m_buffer;
}
} // namespace render::scene
