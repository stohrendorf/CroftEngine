#pragma once

#include "core/vec.h"

namespace engine
{
class ObjectManager;

extern std::pair<bool, core::RoomBoundPosition>
  raycastLineOfSight(const core::RoomBoundPosition& start, const core::TRVec& goal, const ObjectManager& objectManager);
} // namespace engine
