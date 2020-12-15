#pragma once

#include "core/angle.h"
#include "core/units.h"
#include "engine/items_tr1.h"

#include <bitset>
#include <cstdint>
#include <string>

namespace engine
{
class SkeletalModelNode;
}

namespace menu
{
struct MenuRingTransform;
struct MenuObject
{
  std::string name;
  engine::TR1ItemId type;
  core::Frame lastMeshAnimFrame;
  core::Frame openFrame;
  core::Angle selectedBaseRotationX;
  core::Angle selectedRotationX;
  core::Angle selectedRotationY;
  core::Length selectedPositionZ;
  std::bitset<32> defaultMeshRenderMask{0xffffffffull};
  std::bitset<32> meshRenderMask{0xffffffffull};

  core::Frame meshAnimFrame{0_frame};
  core::Frame goalFrame{0_frame};
  core::Frame animDirection{1_frame};
  core::Frame animStretch{1_frame};
  core::Frame animStretchCounter{0_frame};
  core::Angle baseRotationX{0_deg};
  core::Angle rotationX{0_deg};
  core::Angle rotationY{0_deg};
  core::Length positionZ{0_len};
  mutable core::Angle compassNeedleRotation = 0_deg;
  mutable core::Angle compassNeedleRotationMomentum = 0_deg;

  std::shared_ptr<engine::SkeletalModelNode> node;
  void initModel(const engine::World& world);

  bool animate();

  void updateMeshRenderMask();
  void draw(const engine::World& world, const MenuRingTransform& ringTransform, const core::Angle& ringItemAngle) const;
};
} // namespace menu
