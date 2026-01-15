#pragma once

#include "core/units.h"
#include "engine/items_tr1.h"

#include <bitset>
#include <gl/buffer.h>
#include <gslu.h>
#include <memory>
#include <string>

namespace engine::world
{
class World;
}

namespace engine
{
class SkeletalModelNode;
struct ShaderLight;
} // namespace engine

namespace menu
{
struct MenuRingTransform;
struct MenuObject
{
  std::string name;
  engine::TR1ItemId type{-1};
  core::MenuFrame lastMeshAnimFrame;
  core::MenuFrame openFrame;
  core::Angle selectedBaseRotationX;
  core::Angle selectedRotationX;
  core::Angle selectedRotationY;
  core::Length selectedPositionZ;
  std::bitset<32> defaultMeshRenderMask{0xffffffffull};
  std::bitset<32> meshRenderMask{0xffffffffull};

  core::MenuFrame meshAnimFrame{0_mframe};
  core::MenuFrame goalFrame{0_mframe};
  core::MenuFrame animDirection{1_mframe};
  core::MenuFrame animStretch{1_mframe};
  core::MenuFrame animStretchCounter{0_mframe};
  core::Angle baseRotationX{0_deg};
  core::Angle rotationX{0_deg};
  core::Angle rotationY{0_deg};
  core::Length positionZ{0_len};
  mutable core::Angle compassNeedleRotation = 0_deg;
  mutable core::Angle compassNeedleRotationMomentum = 0_deg;

  std::shared_ptr<engine::SkeletalModelNode> node{nullptr};
  void initModel(engine::world::World& world,
                 const gslu::nn_shared<gl::ShaderStorageBuffer<engine::ShaderLight>>& lights);

  bool tick();

  void updateMeshRenderMask();
  void draw(const engine::world::World& world,
            const MenuRingTransform& ringTransform,
            const core::Angle& ringItemAngle) const;
};
} // namespace menu
