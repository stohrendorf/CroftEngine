#include "roomboundposition.h"

#include "engine/world/world.h"
#include "serialization/serialization.h"
#include "serialization/vector_element.h"

namespace engine
{
void RoomBoundPosition::serialize(const serialization::Serializer<world::World>& ser)
{
  ser(S_NV_VECTOR_ELEMENT_NOT_NULL("room", ser.context.getRooms(), room), S_NV("position", position));
}

RoomBoundPosition RoomBoundPosition::create(const serialization::Serializer<world::World>& ser)
{
  const world::Room* room = nullptr;
  core::TRVec position{};
  ser(S_NV_VECTOR_ELEMENT("room", ser.context.getRooms(), room), S_NV("position", position));
  return RoomBoundPosition{room, position};
}

std::ostream& operator<<(std::ostream& stream, const RoomBoundPosition& rhs)
{
  return stream << "[" << rhs.room->node->getName() << " " << rhs.position << "]";
}
} // namespace engine
