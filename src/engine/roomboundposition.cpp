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

gsl::not_null<const world::Sector*> RoomBoundPosition::updateRoom()
{
  const world::Sector* sector;
  while(true)
  {
    sector = room->getBoundarySectorByIndex((position.X - room->position.X) / core::SectorSize,
                                            (position.Z - room->position.Z) / core::SectorSize);
    if(sector->boundaryRoom == nullptr)
    {
      break;
    }

    room = sector->boundaryRoom;
  }

  // go up/down until we are in the room that contains our coordinates
  Expects(sector != nullptr);
  if(position.Y >= sector->floorHeight)
  {
    while(position.Y >= sector->floorHeight && sector->roomBelow != nullptr)
    {
      room = sector->roomBelow;
      sector = room->getSectorByAbsolutePosition(position);
      Expects(sector != nullptr);
    }
  }
  else
  {
    while(position.Y < sector->ceilingHeight && sector->roomAbove != nullptr)
    {
      room = sector->roomAbove;
      sector = room->getSectorByAbsolutePosition(position);
      Expects(sector != nullptr);
    }
  }

  return sector;
}

std::ostream& operator<<(std::ostream& stream, const RoomBoundPosition& rhs)
{
  return stream << "[" << rhs.room->node->getName() << " " << rhs.position << "]";
}
} // namespace engine
