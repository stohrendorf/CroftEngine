#include "secrets.h"

#include "floordata.h"
#include "types.h"

#include <bitset>
#include <boost/assert.hpp>
#include <boost/throw_exception.hpp>
#include <stdexcept>

namespace engine::floordata
{

std::bitset<16> getSecretsMask(const FloorDataValue* floorData)
{
  if(floorData == nullptr)
    return 0;

  std::bitset<16> result{};
  while(true)
  {
    const FloorDataChunk chunkHeader{*floorData++};

    switch(chunkHeader.type)
    {
    case FloorDataChunkType::FloorSlant:
    case FloorDataChunkType::CeilingSlant:
    case FloorDataChunkType::BoundaryRoom:
      ++floorData;
      if(chunkHeader.isLast)
        return result;
      continue;
    case FloorDataChunkType::Death:
      if(chunkHeader.isLast)
        return result;
      continue;
    case FloorDataChunkType::CommandSequence:
      break;
    default:
      BOOST_THROW_EXCEPTION(std::domain_error("unexpected chunk header type"));
    }

    ++floorData;
    while(true)
    {
      const Command command{*floorData++};
      switch(command.opcode)
      {
      case CommandOpcode::SwitchCamera:
        command.isLast = CameraParameters{*floorData++}.isLast;
        break;
      case CommandOpcode::Activate:
      case CommandOpcode::LookAt:
      case CommandOpcode::UnderwaterCurrent:
      case CommandOpcode::FlipMap:
      case CommandOpcode::FlipOn:
      case CommandOpcode::FlipOff:
      case CommandOpcode::FlipEffect:
      case CommandOpcode::EndLevel:
      case CommandOpcode::PlayTrack:
        break;
      case CommandOpcode::Secret:
        BOOST_ASSERT(command.parameter < 16);
        result.set(command.parameter);
        break;
      default:
        BOOST_THROW_EXCEPTION(std::domain_error("unexpected command opcode"));
      }

      if(command.isLast)
        break;
    }

    if(chunkHeader.isLast)
      break;
  }

  return result;
}
} // namespace engine::floordata
