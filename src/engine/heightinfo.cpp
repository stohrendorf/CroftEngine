#include "heightinfo.h"

#include "cameracontroller.h"
#include "engine/objects/object.h"
#include "floordata/floordata.h"

namespace engine
{
bool HeightInfo::skipSteepSlants = false;

HeightInfo HeightInfo::fromFloor(gsl::not_null<const loader::file::Sector*> roomSector,
                                 const core::TRVec& pos,
                                 const std::map<uint16_t, gsl::not_null<std::shared_ptr<objects::Object>>>& objects)
{
  HeightInfo hi;

  while(roomSector->roomBelow != nullptr)
  {
    roomSector = roomSector->roomBelow->getSectorByAbsolutePosition(pos);
  }

  hi.y = roomSector->floorHeight;

  if(roomSector->floorData == nullptr)
  {
    return hi;
  }

  // process additional slant and object height patches
  const floordata::FloorDataValue* fd = roomSector->floorData;
  while(true)
  {
    const floordata::FloorDataChunk chunkHeader{*fd++};
    switch(chunkHeader.type)
    {
    case floordata::FloorDataChunkType::FloorSlant:
    {
      const core::Length::type xSlant = gsl::narrow_cast<int8_t>(fd->get() & 0xffu);
      const core::Length::type absX = std::abs(xSlant);
      const core::Length::type zSlant = gsl::narrow_cast<int8_t>((fd->get() >> 8u) & 0xffu);
      const core::Length::type absZ = std::abs(zSlant);
      if(!skipSteepSlants || (absX <= 2 && absZ <= 2))
      {
        if(absX <= 2 && absZ <= 2)
          hi.slantClass = SlantClass::Max512;
        else
          hi.slantClass = SlantClass::Steep;

        const auto localX = pos.X % core::SectorSize;
        const auto localZ = pos.Z % core::SectorSize;

        if(zSlant > 0) // lower edge at -Z
        {
          const core::Length dist = core::SectorSize - localZ;
          hi.y += dist * zSlant * core::QuarterSectorSize / core::SectorSize;
        }
        else if(zSlant < 0) // lower edge at +Z
        {
          const auto dist = localZ;
          hi.y -= dist * zSlant * core::QuarterSectorSize / core::SectorSize;
        }

        if(xSlant > 0) // lower edge at -X
        {
          const auto dist = core::SectorSize - localX;
          hi.y += dist * xSlant * core::QuarterSectorSize / core::SectorSize;
        }
        else if(xSlant < 0) // lower edge at +X
        {
          const auto dist = localX;
          hi.y -= dist * xSlant * core::QuarterSectorSize / core::SectorSize;
        }
      }
    }
      // Fall-through
    case floordata::FloorDataChunkType::CeilingSlant:
    case floordata::FloorDataChunkType::PortalSector: ++fd; break;
    case floordata::FloorDataChunkType::Death: hi.lastCommandSequenceOrDeath = fd - 1; break;
    case floordata::FloorDataChunkType::CommandSequence:
      if(hi.lastCommandSequenceOrDeath == nullptr)
        hi.lastCommandSequenceOrDeath = fd - 1;
      ++fd;
      while(true)
      {
        const floordata::Command command{*fd++};

        if(command.opcode == floordata::CommandOpcode::Activate)
        {
          objects.at(command.parameter)->patchFloor(pos, hi.y);
        }
        else if(command.opcode == floordata::CommandOpcode::SwitchCamera)
        {
          command.isLast = floordata::CameraParameters{*fd++}.isLast;
        }

        if(command.isLast)
          break;
      }
    default: break;
    }
    if(chunkHeader.isLast)
      break;
  }

  return hi;
}

HeightInfo HeightInfo::fromCeiling(gsl::not_null<const loader::file::Sector*> roomSector,
                                   const core::TRVec& pos,
                                   const std::map<uint16_t, gsl::not_null<std::shared_ptr<objects::Object>>>& objects)
{
  HeightInfo hi;

  while(roomSector->roomAbove != nullptr)
  {
    roomSector = roomSector->roomAbove->getSectorByAbsolutePosition(pos);
  }

  hi.y = roomSector->ceilingHeight;

  if(roomSector->floorData != nullptr)
  {
    const floordata::FloorDataValue* fd = roomSector->floorData;
    floordata::FloorDataChunk chunkHeader{*fd};
    ++fd;

    if(chunkHeader.type == floordata::FloorDataChunkType::FloorSlant)
    {
      ++fd;

      chunkHeader = floordata::FloorDataChunk{*fd};
      ++fd;
    }

    if(chunkHeader.type == floordata::FloorDataChunkType::CeilingSlant)
    {
      const core::Length::type xSlant = gsl::narrow_cast<int8_t>(fd->get() & 0xffu);
      const core::Length::type absX = std::abs(xSlant);
      const core::Length::type zSlant = gsl::narrow_cast<int8_t>((fd->get() >> 8u) & 0xffu);
      const core::Length::type absZ = std::abs(zSlant);
      if(!skipSteepSlants || (absX <= 2 && absZ <= 2))
      {
        const auto localX = pos.X % core::SectorSize;
        const auto localZ = pos.Z % core::SectorSize;

        if(zSlant > 0) // lower edge at -Z
        {
          const auto dist = core::SectorSize - localZ;
          hi.y -= dist * zSlant * core::QuarterSectorSize / core::SectorSize;
        }
        else if(zSlant < 0) // lower edge at +Z
        {
          const auto dist = localZ;
          hi.y += dist * zSlant * core::QuarterSectorSize / core::SectorSize;
        }

        if(xSlant > 0) // lower edge at -X
        {
          const auto dist = localX;
          hi.y -= dist * xSlant * core::QuarterSectorSize / core::SectorSize;
        }
        else if(xSlant < 0) // lower edge at +X
        {
          const auto dist = core::SectorSize - localX;
          hi.y += dist * xSlant * core::QuarterSectorSize / core::SectorSize;
        }
      }
    }
  }

  while(roomSector->roomBelow != nullptr)
  {
    roomSector = roomSector->roomBelow->getSectorByAbsolutePosition(pos);
  }

  if(roomSector->floorData == nullptr)
    return hi;

  const floordata::FloorDataValue* fd = roomSector->floorData;
  while(true)
  {
    const floordata::FloorDataChunk chunkHeader{*fd++};
    switch(chunkHeader.type)
    {
    case floordata::FloorDataChunkType::CeilingSlant:
    case floordata::FloorDataChunkType::FloorSlant:
    case floordata::FloorDataChunkType::PortalSector: ++fd; break;
    case floordata::FloorDataChunkType::Death: break;
    case floordata::FloorDataChunkType::CommandSequence:
      ++fd;
      while(true)
      {
        const floordata::Command command{*fd++};

        if(command.opcode == floordata::CommandOpcode::Activate)
        {
          objects.at(command.parameter)->patchCeiling(pos, hi.y);
        }
        else if(command.opcode == floordata::CommandOpcode::SwitchCamera)
        {
          command.isLast = floordata::CameraParameters{*fd++}.isLast;
        }

        if(command.isLast)
          break;
      }
    default: break;
    }
    if(chunkHeader.isLast)
      break;
  }

  return hi;
}
} // namespace engine
