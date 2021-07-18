#pragma once

#include "core/vec.h"
#include "serialization/serialization_fwd.h"

#include <iosfwd>

namespace engine::world
{
struct Room;
}

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
};

extern std::ostream& operator<<(std::ostream& stream, const RoomBoundPosition& rhs);
} // namespace engine
