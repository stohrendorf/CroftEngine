#pragma once

#include "core/vec.h"
#include "serialization/serialization_fwd.h"

#include <iosfwd>

namespace engine::world
{
struct Room;
struct Sector;
} // namespace engine::world

namespace engine
{
struct RoomBoundPosition final
{
  gsl::not_null<const world::Room*> room;

  core::TRVec position;

  explicit RoomBoundPosition(gsl::not_null<const world::Room*> r, core::TRVec pos = {})
      : room{std::move(r)}
      , position{std::move(pos)}
  {
  }

  void serialize(const serialization::Serializer<world::World>& ser);
  [[nodiscard]] static RoomBoundPosition create(const serialization::Serializer<world::World>& ser);

  gsl::not_null<const world::Sector*> updateRoom();

  [[nodiscard]] auto delta(const core::TRVec& d) const
  {
    return RoomBoundPosition{room, position + d};
  }

  [[nodiscard]] auto delta(const core::Length& dx, const core::Length& dy, const core::Length& dz) const
  {
    return RoomBoundPosition{room, position + core::TRVec{dx, dy, dz}};
  }
};

extern std::ostream& operator<<(std::ostream& stream, const RoomBoundPosition& rhs);
} // namespace engine
