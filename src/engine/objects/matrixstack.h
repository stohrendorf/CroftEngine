#pragma once

#include "core/angle.h"
#include "core/vec.h"
#include "engine/world/skeletalmodeltype.h"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <stack>
#include <vector>

namespace engine::objects
{
class MatrixStack
{
  std::stack<glm::mat4> m_stack;
  std::vector<glm::mat4> m_poseMatrices;

public:
  explicit MatrixStack();

  void push();

  void pop() noexcept;

  [[nodiscard]] const glm::mat4& top() const noexcept;

  glm::mat4& top() noexcept;

  void multiply(const glm::mat4& m);

  void rotate(const core::TRRotation& r);

  void rotate(const core::TRRotationXY& r);

  void resetRotation();

  void rotate(const uint8_t* packed);

  void translate(const glm::vec3& x);

  void transform(const std::initializer_list<size_t>& indices,
                 const std::vector<world::SkeletalModelType::Bone>& bones,
                 const gsl_lite::span<const uint8_t>& angleData);

  void transform(const size_t idx,
                 const std::vector<world::SkeletalModelType::Bone>& bones,
                 const gsl_lite::span<const uint8_t>& angleData);

  void apply(size_t idx);

  [[nodiscard]] const std::vector<glm::mat4>& getPoseMatrices() const noexcept;
};
} // namespace engine::objects