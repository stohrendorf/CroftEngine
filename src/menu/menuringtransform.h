#pragma once

#include "core/angle.h"
#include "core/units.h"
#include "core/vec.h"

#include <glm/gtc/matrix_transform.hpp>

namespace menu
{
struct MenuRingTransform
{
  core::Length radius{0_len};
  core::Angle cameraRotX{};
  core::Angle ringRotation{180_deg};
  core::TRVec cameraPos{0_len, -1536_len, 896_len};

  [[nodiscard]] glm::mat4 getModelMatrix() const
  {
    return core::TRRotation{0_deg, -90_deg - ringRotation, 0_deg}.toMatrix();
  }

  [[nodiscard]] glm::mat4 getView() const
  {
    return glm::lookAt(
             cameraPos.toRenderSystem(), core::TRVec{0_len, 96_len, radius}.toRenderSystem(), glm::vec3{0, 1, 0})
           * glm::eulerAngleX(core::toRad(cameraRotX));
  }
};
} // namespace menu
