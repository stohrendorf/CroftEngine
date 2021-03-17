#pragma once

#include "core/vec.h"

namespace engine::world
{
struct CinematicFrame
{
  core::TRVec lookAt;
  core::TRVec position;
  float fov;
  float rotZ;
};
} // namespace engine::world
