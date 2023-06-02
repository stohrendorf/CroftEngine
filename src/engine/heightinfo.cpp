#include "heightinfo.h"

#include "core/vec.h"
#include "engine/floordata/floordata.h"
#include "engine/objects/object.h"
#include "engine/world/room.h"
#include "engine/world/sector.h"
#include "util/helpers.h"

#include <cstdlib>
#include <gslu.h>
#include <memory>
#include <type_traits>

namespace engine
{
bool HeightInfo::skipSteepSlants = false;

HeightInfo HeightInfo::fromFloor(gsl::not_null<const world::Sector*> roomSector,
                                 const core::TRVec& pos,
                                 const std::map<uint16_t, gslu::nn_shared<objects::Object>>& objects)
{
  HeightInfo hi;

  while(roomSector->roomBelow != nullptr)
  {
    roomSector = gsl::not_null{roomSector->roomBelow->getSectorByAbsolutePosition(pos)};
  }

  hi.y = roomSector->floorHeight;

  if(roomSector->floorData == nullptr)
  {
    return hi;
  }

  // process additional slant and object height patches
  const floordata::FloorDataValue* fd = gsl::not_null{roomSector->floorData}.get();
  while(true)
  {
    const floordata::FloorDataChunk chunkHeader{*fd++};
    switch(chunkHeader.type)
    {
    case floordata::FloorDataChunkType::FloorSlant:
    {
      const core::Length::type xSlant = gsl::narrow_cast<int8_t>(util::bits(fd->get(), 0, 8));
      const core::Length::type zSlant = gsl::narrow_cast<int8_t>(util::bits(fd->get(), 8, 8));
      ++fd;
      const core::Length::type absX = std::abs(xSlant);
      const core::Length::type absZ = std::abs(zSlant);
      if(!skipSteepSlants || (absX <= 2 && absZ <= 2))
      {
        if(absX <= 2 && absZ <= 2)
          hi.slantClass = SlantClass::Max512;
        else
          hi.slantClass = SlantClass::Steep;

        const auto localX = toSectorLocal(pos.X);
        const auto localZ = toSectorLocal(pos.Z);

        if(zSlant > 0) // lower edge at -Z
        {
          const auto dist = 1_sectors - localZ;
          hi.y += dist * zSlant * core::QuarterSectorSize / core::SectorSize;
        }
        else if(zSlant < 0) // lower edge at +Z
        {
          // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
          const auto dist = localZ;
          hi.y -= dist * zSlant * core::QuarterSectorSize / core::SectorSize;
        }

        if(xSlant > 0) // lower edge at -X
        {
          const auto dist = 1_sectors - localX;
          hi.y += dist * xSlant * core::QuarterSectorSize / core::SectorSize;
        }
        else if(xSlant < 0) // lower edge at +X
        {
          // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
          const auto dist = localX;
          hi.y -= dist * xSlant * core::QuarterSectorSize / core::SectorSize;
        }
      }
    }
    break;
      // NOLINTNEXTLINE(bugprone-branch-clone)
    case floordata::FloorDataChunkType::CeilingSlant:
      ++fd;
      break;
    case floordata::FloorDataChunkType::BoundaryRoom:
      ++fd;
      break;
    case floordata::FloorDataChunkType::Death:
      hi.lastCommandSequenceOrDeath = fd - 1;
      break;
    case floordata::FloorDataChunkType::CommandSequence:
      if(hi.lastCommandSequenceOrDeath == nullptr)
        hi.lastCommandSequenceOrDeath = fd - 1;
      ++fd;
      while(true)
      {
        const floordata::Command command{*fd++};

        if(command.opcode == floordata::CommandOpcode::Activate)
        {
          if(auto it = objects.find(command.parameter); it != objects.end())
          {
            it->second->patchFloor(pos, hi.y);
          }
        }
        else if(command.opcode == floordata::CommandOpcode::SwitchCamera)
        {
          command.isLast = floordata::CameraParameters{*fd++}.isLast;
        }

        if(command.isLast)
          break;
      }
      break;
    default:
      break;
    }
    if(chunkHeader.isLast)
      break;
  }

  return hi;
}

HeightInfo HeightInfo::fromCeiling(gsl::not_null<const world::Sector*> roomSector,
                                   const core::TRVec& pos,
                                   const std::map<uint16_t, gslu::nn_shared<objects::Object>>& objects)
{
  HeightInfo hi;

  while(roomSector->roomAbove != nullptr)
  {
    roomSector = gsl::not_null{roomSector->roomAbove->getSectorByAbsolutePosition(pos)};
  }

  hi.y = roomSector->ceilingHeight;

  if(roomSector->floorData != nullptr)
  {
    const floordata::FloorDataValue* fd = gsl::not_null{roomSector->floorData}.get();
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
      const core::Length::type xSlant = gsl::narrow_cast<int8_t>(util::bits(fd->get(), 0, 8));
      const core::Length::type absX = std::abs(xSlant);
      const core::Length::type zSlant = gsl::narrow_cast<int8_t>(util::bits(fd->get(), 8, 8));
      const core::Length::type absZ = std::abs(zSlant);
      if(!skipSteepSlants || (absX <= 2 && absZ <= 2))
      {
        const auto localX = toSectorLocal(pos.X);
        const auto localZ = toSectorLocal(pos.Z);

        if(zSlant > 0) // lower edge at -Z
        {
          const auto dist = 1_sectors - localZ;
          hi.y -= dist * zSlant * core::QuarterSectorSize / core::SectorSize;
        }
        else if(zSlant < 0) // lower edge at +Z
        {
          // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
          const auto dist = localZ;
          hi.y += dist * zSlant * core::QuarterSectorSize / core::SectorSize;
        }

        if(xSlant > 0) // lower edge at -X
        {
          // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
          const auto dist = localX;
          hi.y -= dist * xSlant * core::QuarterSectorSize / core::SectorSize;
        }
        else if(xSlant < 0) // lower edge at +X
        {
          const auto dist = 1_sectors - localX;
          hi.y += dist * xSlant * core::QuarterSectorSize / core::SectorSize;
        }
      }
    }
  }

  while(roomSector->roomBelow != nullptr)
  {
    roomSector = gsl::not_null{roomSector->roomBelow->getSectorByAbsolutePosition(pos)};
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
    case floordata::FloorDataChunkType::BoundaryRoom:
      ++fd;
      break;
    case floordata::FloorDataChunkType::Death:
      break;
    case floordata::FloorDataChunkType::CommandSequence:
      ++fd;
      while(true)
      {
        const floordata::Command command{*fd++};

        if(command.opcode == floordata::CommandOpcode::Activate)
        {
          if(auto it = objects.find(command.parameter); it != objects.end())
          {
            it->second->patchCeiling(pos, hi.y);
          }
        }
        else if(command.opcode == floordata::CommandOpcode::SwitchCamera)
        {
          command.isLast = floordata::CameraParameters{*fd++}.isLast;
        }

        if(command.isLast)
          break;
      }
      break;
    default:
      break;
    }
    if(chunkHeader.isLast)
      break;
  }

  return hi;
}
} // namespace engine
