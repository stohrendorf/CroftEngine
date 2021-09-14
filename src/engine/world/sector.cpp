#include "sector.h"

#include "core/containeroffset.h"
#include "core/id.h"
#include "engine/floordata/floordata.h"
#include "loader/file/datatypes.h"
#include "serialization/optional.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "serialization/vector_element.h"
#include "world.h"

#include <exception>
#include <gsl/gsl-lite.hpp>

namespace engine::world
{
Sector::Sector(const loader::file::Sector& src,
               std::vector<Room>& rooms,
               const std::vector<Box>& boxes,
               const engine::floordata::FloorData& newFloorData)
    : box{src.boxIndex.get() >= 0 ? &boxes.at(src.boxIndex.get()) : nullptr}
    , floorHeight{src.floorHeight}
    , ceilingHeight{src.ceilingHeight}
    , m_roomIndexBelow{src.roomIndexBelow.get() == 0xff ? std::nullopt : std::optional{src.roomIndexBelow.get()}}
    , m_roomIndexAbove{src.roomIndexAbove.get() == 0xff ? std::nullopt : std::optional{src.roomIndexAbove.get()}}
{
  connect(rooms);

  if(src.floorDataIndex.index != 0)
  {
    floorData = &src.floorDataIndex.from(newFloorData);

    if(const auto boundaryRoomIndex = engine::floordata::getBoundaryRoom(floorData); boundaryRoomIndex.has_value())
    {
      boundaryRoom = &rooms.at(*boundaryRoomIndex);
    }
  }
}

void Sector::connect(std::vector<Room>& rooms)
{
  if(m_roomIndexBelow.has_value())
  {
    roomBelow = &rooms.at(*m_roomIndexBelow);
  }
  else
  {
    roomBelow = nullptr;
  }

  if(m_roomIndexAbove.has_value())
  {
    roomAbove = &rooms.at(*m_roomIndexAbove);
  }
  else
  {
    roomAbove = nullptr;
  }
}

void Sector::serialize(const serialization::Serializer<World>& ser)
{
  ser(S_NV_VECTOR_ELEMENT("box", ser.context.getBoxes(), box),
      S_NV("floorHeight", floorHeight),
      S_NV("ceilingHeight", ceilingHeight),
      S_NV("roomIndexBelow", m_roomIndexBelow),
      S_NV("roomIndexAbove", m_roomIndexAbove),
      S_NV_VECTOR_ELEMENT("boundaryRoom", ser.context.getRooms(), boundaryRoom),
      S_NV_VECTOR_ELEMENT("floorData", ser.context.getFloorData(), floorData));

  if(ser.loading)
  {
    ser.lazy([this](const serialization::Serializer<World>& ser) { connect(ser.context.getRooms()); });
  }
}
} // namespace engine::world
