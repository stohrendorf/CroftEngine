#include "csm.h"

#include "camera.h"

#include <numeric>

namespace render::scene
{
void CSM::Split::init(int32_t resolution, size_t idx)
{
  texture = std::make_shared<gl::TextureDepth<float>>("csm-texture/" + std::to_string(idx));
  texture->allocate(glm::ivec2{resolution, resolution})
    .fill(1.0f)
    .set(::gl::TextureMinFilter::Linear)
    .set(::gl::TextureMagFilter::Linear)
    .set(::gl::TextureCompareMode::CompareRefToTexture)
    .set(::gl::DepthFunction::Gequal)
    .set(::gl::TextureParameterName::TextureWrapS, ::gl::TextureWrapMode::ClampToBorder)
    .set(::gl::TextureParameterName::TextureWrapT, ::gl::TextureWrapMode::ClampToBorder)
    .setBorderColor(glm::vec4{1.0f});

  framebuffer = gl::FrameBufferBuilder()
                  .textureNoBlend(::gl::FramebufferAttachment::DepthAttachment, texture)
                  .build("csm-split-fb/" + std::to_string(idx));
}

CSM::CSM(int32_t resolution)
    : m_resolution{resolution}
{
  static_assert(CSMBuffer::NSplits > 0);
  Expects(resolution > 0);

  for(size_t i = 0; i < CSMBuffer::NSplits; ++i)
  {
    m_splits[i].init(resolution, i);
  }
}

std::array<std::shared_ptr<gl::TextureDepth<float>>, CSMBuffer::NSplits> CSM::getTextures() const
{
  std::array<std::shared_ptr<gl::TextureDepth<float>>, CSMBuffer::NSplits> result{};
  std::transform(m_splits.begin(), m_splits.end(), result.begin(), [](const Split& split) { return split.texture; });
  return result;
}

std::array<glm::mat4, CSMBuffer::NSplits> CSM::getMatrices(const glm::mat4& modelMatrix) const
{
  std::array<glm::mat4, CSMBuffer::NSplits> result{};
  std::transform(m_splits.begin(), m_splits.end(), result.begin(), [modelMatrix](const Split& split) {
    return split.vpMatrix * modelMatrix;
  });
  return result;
}

std::array<float, CSMBuffer::NSplits> CSM::getSplitEnds() const
{
  std::array<float, CSMBuffer::NSplits> result{};
  std::transform(m_splits.begin(), m_splits.end(), result.begin(), [](const Split& split) { return split.end; });
  return result;
}

void CSM::update(const Camera& camera)
{
  //Start off by calculating the split distances
  const float nearClip = camera.getNearPlane();
  const float farClip = camera.getFarPlane();

  std::vector<float> cascadeSplits;
  cascadeSplits.emplace_back(nearClip);
#if 0
  for(size_t i = 0; i < m_splits.size(); ++i)
  {
    static constexpr float Lambda = 1.0f;
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
    m_splits[i].end = -zi;
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
    const auto lightViewWS = glm::lookAt(position - m_lightDir, position, m_lightDirOrtho);

    glm::vec3 bboxMin{std::numeric_limits<float>::max()};
    glm::vec3 bboxMax{std::numeric_limits<float>::lowest()};
    for(const auto& corner : frustumCorners)
    {
      const auto cornerLS = glm::vec3{lightViewWS * glm::vec4{corner, 1.0f}};
      for(int i : {0, 1, 2})
      {
        bboxMin[i] = std::min(bboxMin[i], cornerLS[i]);
        bboxMax[i] = std::max(bboxMax[i], cornerLS[i]);
      }
    }

    // extend the bboxes and snap to a grid to avoid some flickering
    static constexpr float SnapSize = 16.0f;
    for(int i : {0, 1, 2})
    {
      bboxMin[i] = glm::floor(bboxMin[i] / SnapSize) * SnapSize;
      bboxMax[i] = glm::ceil(bboxMax[i] / SnapSize) * SnapSize;
    }

    const auto lightProjection = glm::ortho(bboxMin.x, bboxMax.x, bboxMin.y, bboxMax.y, bboxMin.z, bboxMax.z);
    m_splits[cascadeIterator].vpMatrix = lightProjection * lightViewWS;
  }
}
} // namespace render::scene
