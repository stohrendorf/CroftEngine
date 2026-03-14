#pragma once

#include "core/angle.h"
#include "core/vec.h"
#include "matrixstack.h"

#include <cstdint>
#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>

namespace engine
{
}

namespace engine::objects
{
class SegmentInterpolationMatrixStack
{
  MatrixStack m_stack1;
  MatrixStack m_stack2;
  float m_interKeyframeFactor;

public:
  explicit SegmentInterpolationMatrixStack(const float interKeyframeFactor);

  void push();

  void pop();

  [[nodiscard]] glm::mat4 itop() const;

  void rotate(const glm::mat4& m);

  void rotate(const core::TRRotation& r);

  void rotate(const core::TRRotationXY& r);

  void rotate(const uint8_t* packed1, const uint8_t* packed2);

  void resetRotation();

  void translate(const glm::vec3& v1, const glm::vec3& v2);

  void translate(const glm::vec3& v);

  void transform(const std::initializer_list<size_t>& indices,
                 const std::vector<world::SkeletalModelType::Bone>& bones,
                 const gsl_lite::span<const uint8_t>& angleData1,
                 const gsl_lite::span<const uint8_t>& angleData2);

  void transform(const size_t idx,
                 const std::vector<world::SkeletalModelType::Bone>& bones,
                 const gsl_lite::span<const uint8_t>& angleData1,
                 const gsl_lite::span<const uint8_t>& angleData2);

  // NOLINTNEXTLINE(readability-make-member-function-const)
  void apply(size_t idx);

  std::vector<glm::mat4> getPoseMatrices() const;
};
} // namespace engine::objects