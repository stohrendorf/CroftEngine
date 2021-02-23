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

extern std::pair<bool, core::RoomBoundPosition> raycastLineOfSight(const core::RoomBoundPosition& firstStepAxis,
                                                                   const core::TRVec& secondStepAxis,
                                                                   const ObjectManager& objectManager);
} // namespace engine
