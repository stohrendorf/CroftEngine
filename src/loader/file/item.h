#pragma once

#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"

#include <cstdint>
#include <memory>

namespace loader::file
{
namespace io
{
class SDLReader;
}

struct Item
{
  core::TypeId type{
    uint16_t(0)}; //!< Object Identifier (matched in AnimatedModels[], or SpriteSequences[], as appropriate)
  core::RoomId16 room{uint16_t(0)}; //!< Owning room
  core::TRVec position;             //!< world coords
  core::Angle rotation;             //!< ((0xc000 >> 14) * 90) degrees around Y axis
  core::Shade shade;                //!< (constant lighting; -1 means use mesh lighting)
  int16_t intensity2 = 0; //!< Like Intensity1, and almost always with the same value. [absent from TR1 data files]
  int16_t ocb = 0;        //!< Object code bit - used for altering entity behaviour. Only in TR4-5.

  uint16_t activationState = 0;

  static std::unique_ptr<Item> readTr1(io::SDLReader& reader);
  static std::unique_ptr<Item> readTr2(io::SDLReader& reader);
  static std::unique_ptr<Item> readTr3(io::SDLReader& reader);
  static std::unique_ptr<Item> readTr4(io::SDLReader& reader);
};
} // namespace loader::file
