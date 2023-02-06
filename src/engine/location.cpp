#include "location.h"

#include "core/magic.h"
#include "engine/world/world.h"
#include "qs/qs.h"
#include "render/scene/node.h"
#include "serialization/serialization.h"
#include "serialization/vector_element.h"
#include "world/room.h"
#include "world/sector.h"

#include <exception>
#include <memory>
#include <ostream>
#include <string>

namespace engine
{
void Location::serialize(const serialization::Serializer<world::World>& ser) const
{
  ser(S_NV_VECTOR_ELEMENT_NOT_NULL_SERIALIZE("room", ser.context.getRooms(), room), S_NV("position", position));
}

void Location::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ser(S_NV_VECTOR_ELEMENT_NOT_NULL_DESERIALIZE("room", ser.context.getRooms(), room), S_NV("position", position));
}

Location Location::create(const serialization::Deserializer<world::World>& ser)
{
  const world::Room* room = nullptr;
  core::TRVec position{};
  ser(S_NV_VECTOR_ELEMENT("room", std::cref(ser.context.getRooms()), std::ref(room)), S_NV("position", position));
  return Location{gsl::not_null{room}, position};
}

gsl::not_null<const world::Sector*> Location::updateRoom()
{
  const world::Sector* sector;
  while(true)
  {
    sector = room->getBoundarySectorByIndex(sectorOf(position.X - room->position.X),
                                            sectorOf(position.Z - room->position.Z));
    if(sector->boundaryRoom == nullptr)
    {
      break;
    }

    room = gsl::not_null{sector->boundaryRoom};
  }

  // go up/down until we are in the room that contains our coordinates
  gsl_Assert(sector != nullptr);
  if(position.Y >= sector->floorHeight)
  {
    while(position.Y >= sector->floorHeight && sector->roomBelow != nullptr)
    {
      room = gsl::not_null{sector->roomBelow};
      sector = room->getSectorByAbsolutePosition(position);
      gsl_Assert(sector != nullptr);
    }
  }
  else
  {
    while(position.Y < sector->ceilingHeight && sector->roomAbove != nullptr)
    {
      room = gsl::not_null{sector->roomAbove};
      sector = room->getSectorByAbsolutePosition(position);
      gsl_Assert(sector != nullptr);
    }
  }

  return gsl::not_null{sector};
}

bool Location::isValid() const
{
  return room->isInnerPositionXZ(position);
}

const world::Sector* Location::getCurrentSector() const
{
  return room->getSectorByAbsolutePosition(position);
}

std::ostream& operator<<(std::ostream& stream, const Location& rhs)
{
  return stream << "[" << rhs.room->node->getName() << " " << rhs.position << "]";
}
} // namespace engine
