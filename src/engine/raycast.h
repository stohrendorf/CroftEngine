#pragma once

#include <utility>

namespace core
{
struct TRVec;
} // namespace core

namespace engine
{
class ObjectManager;
struct Location;

extern std::pair<bool, Location>
  raycastLineOfSight(const Location& start, const core::TRVec& goal, const ObjectManager& objectManager);
} // namespace engine
