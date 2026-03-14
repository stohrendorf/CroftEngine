#include "segmentinterpolationmatrixstack.h"

#include "util/helpers.h"

namespace engine::objects
{

SegmentInterpolationMatrixStack::SegmentInterpolationMatrixStack(const float interKeyframeFactor)
    : m_interKeyframeFactor{interKeyframeFactor}
{
}

void SegmentInterpolationMatrixStack::push()
{
  m_stack1.push();
  m_stack2.push();
}

void SegmentInterpolationMatrixStack::pop()
{
  m_stack1.pop();
  m_stack2.pop();
}

glm::mat4 SegmentInterpolationMatrixStack::itop() const
{
  return util::lerp(m_stack1.top(), m_stack2.top(), m_interKeyframeFactor);
}

void SegmentInterpolationMatrixStack::rotate(const glm::mat4& m)
{
  m_stack1.top() *= m;
  m_stack2.top() *= m;
}

void SegmentInterpolationMatrixStack::rotate(const core::TRRotation& r)
{
  rotate(r.toMatrix());
}

void SegmentInterpolationMatrixStack::rotate(const core::TRRotationXY& r)
{
  rotate(r.toMatrix());
}

void SegmentInterpolationMatrixStack::rotate(const uint8_t* packed1, const uint8_t* packed2)
{
  m_stack1.top() *= core::fromPackedAngles(packed1);
  m_stack2.top() *= core::fromPackedAngles(packed2);
}

void SegmentInterpolationMatrixStack::resetRotation()
{
  m_stack1.resetRotation();
  m_stack2.resetRotation();
}

void SegmentInterpolationMatrixStack::translate(const glm::vec3& v1, const glm::vec3& v2)
{
  m_stack1.top() = glm::translate(m_stack1.top(), v1);
  m_stack2.top() = glm::translate(m_stack2.top(), v2);
}

void SegmentInterpolationMatrixStack::translate(const glm::vec3& v)
{
  translate(v, v);
}

void SegmentInterpolationMatrixStack::transform(const std::initializer_list<size_t>& indices,
                                                const std::vector<world::SkeletalModelType::Bone>& bones,
                                                const gsl_lite::span<const uint8_t>& angleData1,
                                                const gsl_lite::span<const uint8_t>& angleData2)
{
  for(const auto idx : indices)
    transform(idx, bones, angleData1, angleData2);
}

void SegmentInterpolationMatrixStack::transform(const size_t idx,
                                                const std::vector<world::SkeletalModelType::Bone>& bones,
                                                const gsl_lite::span<const uint8_t>& angleData1,
                                                const gsl_lite::span<const uint8_t>& angleData2)
{
  BOOST_ASSERT(idx > 0);
  translate(bones.at(idx).position);
  rotate(&angleData1[sizeof(uint32_t) * idx], &angleData2[sizeof(uint32_t) * idx]);
  apply(idx);
}

void SegmentInterpolationMatrixStack::apply(const size_t idx)
{
  m_stack1.apply(idx);
  m_stack2.apply(idx);
}

std::vector<glm::mat4> SegmentInterpolationMatrixStack::getPoseMatrices() const
{
  const auto matrices1 = m_stack1.getPoseMatrices();
  const auto matrices2 = m_stack2.getPoseMatrices();
  gsl_Assert(matrices1.size() == matrices2.size());

  std::vector<glm::mat4> result;
  for(size_t i = 0; i < matrices1.size(); ++i)
    result.emplace_back(util::lerp(matrices1[i], matrices2[i], m_interKeyframeFactor));
  return result;
}
} // namespace engine::objects
