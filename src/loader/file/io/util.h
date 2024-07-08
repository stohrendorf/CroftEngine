#pragma once

#include "core/vec.h"
#include "sdlreader.h"

#include <cmath>
#include <gsl/gsl-lite.hpp>

namespace loader::file::io
{
inline core::TRVec readCoordinates16(SDLReader& reader)
{
  core::TRVec vertex;
  vertex.X = core::Length{gsl::narrow_cast<core::Length::type>(reader.readI16())};
  vertex.Y = core::Length{gsl::narrow_cast<core::Length::type>(reader.readI16())};
  vertex.Z = core::Length{gsl::narrow_cast<core::Length::type>(reader.readI16())};
  return vertex;
}

inline core::TRVec readCoordinates32(SDLReader& reader)
{
  core::TRVec vertex;
  vertex.X = core::Length{gsl::narrow_cast<core::Length::type>(reader.readI32())};
  vertex.Y = core::Length{gsl::narrow_cast<core::Length::type>(reader.readI32())};
  vertex.Z = core::Length{gsl::narrow_cast<core::Length::type>(reader.readI32())};
  return vertex;
}

inline core::TRVec readCoordinatesF(SDLReader& reader)
{
  core::TRVec vertex;
  vertex.X = core::Length{gsl::narrow_cast<core::Length::type>(std::lround(reader.readF()))};
  vertex.Y = core::Length{gsl::narrow_cast<core::Length::type>(std::lround(reader.readF()))};
  vertex.Z = core::Length{gsl::narrow_cast<core::Length::type>(std::lround(reader.readF()))};
  return vertex;
}
} // namespace loader::file::io
