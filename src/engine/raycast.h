#pragma once

#include "core/vec.h"

#include <utility>

namespace engine
{
class ObjectManager;
struct Location;

extern std::pair<bool, Location>
  raycastLineOfSight(const Location& start, const core::TRVec& goal, const ObjectManager& objectManager);
} // namespace engine
