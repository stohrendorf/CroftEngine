#include "matrixstack.h"

#include <glm/ext/matrix_transform.hpp>

namespace engine::objects
{
MatrixStack::MatrixStack()
{
  m_stack.emplace(1.0f);
}

void MatrixStack::push()
{
  const auto top = m_stack.top();
  m_stack.emplace(top);
}

void MatrixStack::pop() noexcept
{
  m_stack.pop();
}

void MatrixStack::multiply(const glm::mat4& m)
{
  m_stack.top() *= m;
}

void MatrixStack::rotate(const core::TRRotation& r)
{
  multiply(r.toMatrix());
}

void MatrixStack::rotate(const core::TRRotationXY& r)
{
  multiply(r.toMatrix());
}

void MatrixStack::resetRotation()
{
  m_stack.top()[0] = glm::vec4{1, 0, 0, 0};
  m_stack.top()[1] = glm::vec4{0, 1, 0, 0};
  m_stack.top()[2] = glm::vec4{0, 0, 1, 0};
}

void MatrixStack::rotate(const uint8_t* packed)
{
  m_stack.top() *= core::fromPackedAngles(packed);
}

void MatrixStack::translate(const glm::vec3& x)
{
  m_stack.top() = glm::translate(m_stack.top(), x);
}

void MatrixStack::transform(const std::initializer_list<size_t>& indices,
                            const std::vector<world::SkeletalModelType::Bone>& bones,
                            const gsl_lite::span<const uint8_t>& angleData)
{
  for(const auto idx : indices)
    transform(idx, bones, angleData);
}

void MatrixStack::transform(const size_t idx,
                            const std::vector<world::SkeletalModelType::Bone>& bones,
                            const gsl_lite::span<const uint8_t>& angleData)
{
  BOOST_ASSERT(idx > 0);
  translate(bones.at(idx).position);
  rotate(&angleData[sizeof(uint32_t) * idx]);
  apply(idx);
}

void MatrixStack::apply(const size_t idx)
{
  if(idx >= m_poseMatrices.size())
    m_poseMatrices.resize(idx + 1);
  m_poseMatrices[idx] = m_stack.top();
}

const std::vector<glm::mat4>& MatrixStack::getPoseMatrices() const noexcept
{
  return m_poseMatrices;
}

glm::mat4& MatrixStack::top() noexcept
{
  return m_stack.top();
}

const glm::mat4& MatrixStack::top() const noexcept
{
  return m_stack.top();
}
} // namespace engine::objects