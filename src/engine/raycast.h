#pragma once

#include <utility>

namespace core
{
struct TRVec;
struct RoomBoundPosition;
} // namespace core

namespace engine
{
class ObjectManager;

extern std::pair<bool, core::RoomBoundPosition>
  raycastLineOfSight(const core::RoomBoundPosition& start, const core::TRVec& goal, const ObjectManager& objectManager);
} // namespace engine
