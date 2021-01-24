#include "vec.h"

#include "serialization/not_null.h"
#include "serialization/quantity.h"
#include "serialization/room_ptr.h"

namespace core
{
void RoomBoundPosition::serialize(const serialization::Serializer& ser)
{
  ser(S_NV("room", room), S_NV("position", position));
}

RoomBoundPosition RoomBoundPosition::create(const serialization::Serializer& ser)
{
  const loader::file::Room* room = nullptr;
  TRVec position{};
  ser(S_NV("room", room), S_NV("position", position));
  return RoomBoundPosition{room, position};
}

void TRVec::serialize(const serialization::Serializer& ser)
{
  ser(S_NV("x", X), S_NV("y", Y), S_NV("z", Z));
}

std::ostream& operator<<(std::ostream& stream, const TRVec& rhs)
{
  return stream << "(" << rhs.X << ", " << rhs.Y << ", " << rhs.Z << ")";
}

std::ostream& operator<<(std::ostream& stream, const RoomBoundPosition& rhs)
{
  return stream << "[" << rhs.room->node->getName() << " " << rhs.position << "]";
}
} // namespace core
