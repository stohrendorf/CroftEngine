#pragma once

#include "core/angle.h"
#include "core/units.h"
#include "core/vec.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/mat4x4.hpp>

namespace menu
{
struct MenuRingTransform
{
  static constexpr auto CameraZPosOffset = 598_len;
  static constexpr core::Length IdleCameraYPosOffset = -240_len;
  static constexpr auto IdleCameraZPosOffset = 688_len;
  static constexpr auto CameraBackpackZPos = 3000_len; // original value: 896
  static constexpr auto CameraBackpackXRot = 90_deg;
  static constexpr auto CameraSwitchRingXRot = 45_deg;
  static constexpr auto RingOffScreenCameraPosY = 1536_len;

  core::Length radius{0_len};
  core::Angle cameraRotX{CameraBackpackXRot};
  core::Angle ringRotation{180_deg};
  core::TRVec cameraPos{0_len, 0_len, CameraBackpackZPos};

  [[nodiscard]] glm::mat4 getModelMatrix() const
  {
    return core::TRRotation{0_deg, -90_deg - ringRotation, 0_deg}.toMatrix();
  }

  [[nodiscard]] glm::mat4 getView() const
  {
    return glm::lookAt(
             cameraPos.toRenderSystem(), core::TRVec{0_len, cameraPos.Y, radius}.toRenderSystem(), glm::vec3{0, 1, 0})
           * glm::eulerAngleX(core::toRad(-cameraRotX));
  }
};
} // namespace menu
