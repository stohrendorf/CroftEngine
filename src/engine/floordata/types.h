#pragma once

#include "type_safe/integer.hpp"

#include <vector>

namespace engine::floordata
{
using FloorDataValue = type_safe::integer<uint16_t>;
using FloorData = std::vector<FloorDataValue>;

//! @brief Native TR floor data functions
//! @ingroup native
enum class FloorDataChunkType : uint8_t
{
  PortalSector = 0x01,
  FloorSlant = 0x02,
  CeilingSlant = 0x03,
  CommandSequence = 0x04,
  Death = 0x05,
  Climb = 0x06,
  FloorTriangleNW = 0x07,           //  [_\_]
  FloorTriangleNE = 0x08,           //  [_/_]
  CeilingTriangleNW = 0x09,         //  [_/_]
  CeilingTriangleNE = 0x0A,         //  [_\_]
  FloorTriangleNWPortalSW = 0x0B,   //  [P\_]
  FloorTriangleNWPortalNE = 0x0C,   //  [_\P]
  FloorTriangleNEPortalSE = 0x0D,   //  [_/P]
  FloorTriangleNEPortalNW = 0x0E,   //  [P/_]
  CeilingTriangleNWPortalSW = 0x0F, //  [P\_]
  CeilingTriangleNWPortalNE = 0x10, //  [_\P]
  CeilingTriangleNEPortalNW = 0x11, //  [P/_]
  CeilingTriangleNEPortalSE = 0x12, //  [_/P]
  Monkey = 0x13,
  MinecartLeft = 0x14, // In TR3 only. Function changed in TR4+.
  MinecartRight = 0x15 // In TR3 only. Function changed in TR4+.
};

//! @brief Native trigger types.
//! @ingroup native
//! @see FloorDataChunkType::Always
//! @see CommandOpcode
enum class SequenceCondition
{
  LaraIsHere = 0x00,           //!< If Lara is in sector, run (any case).
  LaraOnGround = 0x01,         //!< If Lara is in sector, run (land case).
  ItemActivated = 0x02,        //!< If item is activated, run, else stop.
  KeyUsed = 0x03,              //!< If item is activated, run.
  ItemPickedUp = 0x04,         //!< If item is picked up, run.
  ItemIsHere = 0x05,           //!< If item is in sector, run, else stop.
  LaraOnGroundInverted = 0x06, //!< If Lara is in sector, stop (land case).
  LaraInCombatMode = 0x07,     //!< If Lara is in combat state, run (any case).
  Dummy = 0x08,                //!< If Lara is in sector, run (air case).
  AntiTrigger = 0x09,          //!< TR2-5 only: If Lara is in sector, stop (any case).
  HeavySwitch = 0x0A,          //!< TR3-5 only: If item is activated by item, run.
  HeavyAntiTrigger = 0x0B,     //!< TR3-5 only: If item is activated by item, stop.
  Monkey = 0x0C,               //!< TR3-5 only: If Lara is monkey-swinging, run.
  Skeleton = 0x0D,             //!< TR5 only: Activated by skeleton only?
  TightRope = 0x0E,            //!< TR5 only: If Lara is on tightrope, run.
  CrawlDuck = 0x0F,            //!< TR5 only: If Lara is crawling, run.
  Climb = 0x10,                //!< TR5 only: If Lara is climbing, run.
};

//! @brief Native trigger function types.
//! @ingroup native
//! @see FloorDataChunkType::Always
//! @see SequenceCondition
enum class CommandOpcode
{
  Activate = 0x00,
  SwitchCamera = 0x01,
  UnderwaterCurrent = 0x02,
  FlipMap = 0x03,
  FlipOn = 0x04,
  FlipOff = 0x05,
  LookAt = 0x06,
  EndLevel = 0x07,
  PlayTrack = 0x08,
  FlipEffect = 0x09,
  Secret = 0x0A,
  ClearBodies = 0x0B, // Unused in TR4
  FlyBy = 0x0C,
  CutScene = 0x0D
};
} // namespace engine::floordata