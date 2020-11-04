#pragma once

#include "core/angle.h"
#include "core/units.h"
#include "engine/items_tr1.h"

#include <bitset>
#include <cstdint>
#include <string>

namespace menu
{
struct MenuRingTransform;
struct MenuObject
{
  std::string name;
  engine::TR1ItemId type;
  core::Frame lastMeshAnimFrame;
  core::Frame meshAnimFrame;
  core::Frame goalFrame;
  core::Frame openFrame;
  core::Frame animDirection;
  core::Frame animStretch;
  core::Frame animStretchCounter;
  core::Angle selectedBaseRotationX;
  core::Angle baseRotationX;
  core::Angle selectedRotationX;
  core::Angle rotationX;
  core::Angle selectedRotationY;
  core::Angle rotationY;
  core::Length selectedPositionZ;
  core::Length positionZ;
  std::bitset<32> defaultMeshRenderMask;
  std::bitset<32> meshRenderMask;

  mutable core::Angle compassNeedleRotation = 0_deg;
  mutable core::Angle compassNeedleRotationMomentum = 0_deg;

  bool animate();

  void updateMeshRenderMask();
  void
    draw(const engine::Engine& engine, const MenuRingTransform& ringTransform, const core::Angle& ringItemAngle) const;
};
} // namespace menu
