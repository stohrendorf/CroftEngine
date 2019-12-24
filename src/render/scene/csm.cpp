#include "csm.h"

#include "camera.h"

#include <numeric>

namespace render::scene
{
CSM::Split::Split(int32_t resolution)
{
  texture->image(resolution, resolution)
    .set(::gl::TextureMinFilter::Linear)
    .set(::gl::TextureMagFilter::Linear)
    .set(::gl::TextureParameterName::TextureWrapS, ::gl::TextureWrapMode::ClampToEdge)
    .set(::gl::TextureParameterName::TextureWrapT, ::gl::TextureWrapMode::ClampToEdge);
  framebuffer = gl::FrameBufferBuilder().textureNoBlend(::gl::FramebufferAttachment::DepthAttachment, texture).build();
}

CSM::CSM(uint8_t splits, int32_t resolution)
    : m_resolution{resolution}
{
  Expects(splits > 0);
  Expects(resolution > 0);
  m_splits.reserve(splits);
  for(size_t i = 0; i < splits; ++i)
  {
    m_splits.emplace_back(resolution);
  }
}

void CSM::applyViewport()
{
  GL_ASSERT(::gl::viewport(0, 0, m_resolution, m_resolution));
}
std::vector<std::shared_ptr<gl::TextureDepth>> CSM::getTextures() const
{
  std::vector<std::shared_ptr<gl::TextureDepth>> result;
  std::transform(
    m_splits.begin(), m_splits.end(), std::back_inserter(result), [](const Split& split) { return split.texture; });
  return result;
}

std::vector<glm::mat4> CSM::getMatrices(const glm::mat4& modelMatrix) const
{
  std::vector<glm::mat4> result;
  std::transform(m_splits.begin(), m_splits.end(), std::back_inserter(result), [modelMatrix](const Split& split) {
    return split.pvMatrix * modelMatrix;
  });
  return result;
}

std::vector<float> CSM::getSplitEnds() const
{
  std::vector<float> result;
  std::transform(
    m_splits.begin(), m_splits.end(), std::back_inserter(result), [](const Split& split) { return split.end; });
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

  for(size_t cascadeIterator = 0; cascadeIterator < m_splits.size(); ++cascadeIterator)
  {
    const auto position = camera.getPosition();
    const auto up = camera.getUpVector();
    const auto right = camera.getRightVector();
    const auto forward = camera.getFrontVector();

    const auto nc = position + forward * cascadeSplits[cascadeIterator];
    const auto fc = position + forward * cascadeSplits[cascadeIterator + 1];

    const auto Hnear = glm::tan(camera.getFieldOfViewY() / 2) * cascadeSplits[cascadeIterator];
    const auto Wnear = Hnear * camera.getAspectRatio();
    const auto Hfar = glm::tan(camera.getFieldOfViewY() / 2) * cascadeSplits[cascadeIterator + 1];
    const auto Wfar = Hfar * camera.getAspectRatio();

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

    glm::vec3 bboxMin{std::numeric_limits<float>::max()};
    glm::vec3 bboxMax{std::numeric_limits<float>::lowest()};
    for(const auto& corner : frustumCorners)
    {
      bboxMin.x = std::min(bboxMin.x, corner.x);
      bboxMin.y = std::min(bboxMin.y, corner.y);
      bboxMin.z = std::min(bboxMin.z, corner.z);
      bboxMax.x = std::max(bboxMax.x, corner.x);
      bboxMax.y = std::max(bboxMax.y, corner.y);
      bboxMax.z = std::max(bboxMax.z, corner.z);
    }

    const auto ctr = (bboxMin + bboxMax) / 2.0f;
    float radius = 0;
    radius = std::max(radius, ctr.x - bboxMin.x);
    radius = std::max(radius, ctr.y - bboxMin.y);
    radius = std::max(radius, ctr.z - bboxMin.z);

    const auto eye = ctr - glm::normalize(m_lightDir) * radius;
    const auto lightView = glm::lookAt(eye, ctr, m_lightDirOrtho);
    const auto lightProjection = glm::ortho(-radius, radius, -radius, radius, 0.0f, 2 * radius);

    m_splits[cascadeIterator].pvMatrix = lightProjection * lightView;
  }
}
} // namespace render::scene
