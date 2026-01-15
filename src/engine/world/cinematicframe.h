#pragma once

#include "core/vec.h"

namespace engine::world
{
struct CinematicFrame
{
  core::TRVec lookAt;
  core::TRVec position;
  core::Radians fov;
  core::Radians rotZ;
};
} // namespace engine::world
