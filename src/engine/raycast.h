#pragma once

#include <utility>

namespace core
{
struct TRVec;
} // namespace core

namespace engine
{
class ObjectManager;
struct RoomBoundPosition;

extern std::pair<bool, RoomBoundPosition>
  raycastLineOfSight(const RoomBoundPosition& start, const core::TRVec& goal, const ObjectManager& objectManager);
} // namespace engine
