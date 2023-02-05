#pragma once

#include "core/angle.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/location.h"
#include "modelobject.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "serialization/serialization_fwd.h"

#include <gsl/gsl-lite.hpp>
#include <optional>
#include <string>

namespace engine::world
{
class World;
struct Room;
struct SkeletalModelType;
} // namespace engine::world

namespace loader::file
{
struct Item;
}

namespace engine::objects
{
class SlopedBridge : public ModelObject
{
private:
  int m_flatness;

public:
  SlopedBridge(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
      , m_flatness{0}
  {
  }

  SlopedBridge(const std::string& name,
               const gsl::not_null<world::World*>& world,
               const gsl::not_null<const world::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const world::SkeletalModelType*>& animatedModel,
               const int flatness)
      : ModelObject{name, world, room, item, false, animatedModel, false}
      , m_flatness{flatness}
  {
  }

  void patchFloor(const core::TRVec& pos, core::Length& y) final
  {
    if(!core::isSameSector(pos, m_state.location.position))
      return;

    const auto tmp = m_state.location.position.Y + getBridgeSlopeHeight(pos) / m_flatness;
    if(pos.Y > tmp)
      return;

    y = tmp;
  }

  void patchCeiling(const core::TRVec& pos, core::Length& y) final
  {
    if(!core::isSameSector(pos, m_state.location.position))
      return;

    const auto tmp = m_state.location.position.Y + getBridgeSlopeHeight(pos) / m_flatness;
    if(pos.Y <= tmp)
      return;

    y = tmp + core::QuarterSectorSize;
  }

  void serialize(const serialization::Serializer<world::World>& ser) const override;
  void deserialize(const serialization::Deserializer<world::World>& ser) override;

private:
  core::Length getBridgeSlopeHeight(const core::TRVec& pos) const
  {
    auto axis = axisFromAngle(m_state.rotation.Y, 1_deg);
    gsl_Assert(axis.has_value());

    switch(*axis)
    {
    case core::Axis::Deg0:
      return 1_sectors - 1_len - toSectorLocal(pos.X);
    case core::Axis::Right90:
      return toSectorLocal(pos.Z);
    case core::Axis::Deg180:
      return toSectorLocal(pos.X);
    case core::Axis::Left90:
      return 1_sectors - 1_len - toSectorLocal(pos.Z);
    default:
      return 0_len;
    }
  }
};

class BridgeSlope1 final : public SlopedBridge
{
public:
  BridgeSlope1(const gsl::not_null<world::World*>& world, const Location& location)
      : SlopedBridge{world, location}
  {
  }

  BridgeSlope1(const std::string& name,
               const gsl::not_null<world::World*>& world,
               const gsl::not_null<const world::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : SlopedBridge{name, world, room, item, animatedModel, 4}
  {
  }
};

class BridgeSlope2 final : public SlopedBridge
{
public:
  BridgeSlope2(const gsl::not_null<world::World*>& world, const Location& location)
      : SlopedBridge{world, location}
  {
  }

  BridgeSlope2(const std::string& name,
               const gsl::not_null<world::World*>& world,
               const gsl::not_null<const world::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : SlopedBridge{name, world, room, item, animatedModel, 2}
  {
  }
};
} // namespace engine::objects
