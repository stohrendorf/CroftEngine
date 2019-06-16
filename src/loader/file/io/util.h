#pragma once

#include "core/vec.h"
#include "sdlreader.h"

namespace loader
{
namespace file
{
namespace io
{
inline core::TRVec readCoordinates16(SDLReader& reader)
{
    core::TRVec vertex;
    vertex.X = core::Length{static_cast<core::Length::type>(reader.readI16())};
    vertex.Y = core::Length{static_cast<core::Length::type>(reader.readI16())};
    vertex.Z = core::Length{static_cast<core::Length::type>(reader.readI16())};
    return vertex;
}

inline core::TRVec readCoordinates32(SDLReader& reader)
{
    core::TRVec vertex;
    vertex.X = core::Length{reader.readI32()};
    vertex.Y = core::Length{reader.readI32()};
    vertex.Z = core::Length{reader.readI32()};
    return vertex;
}

inline core::TRVec readCoordinatesF(SDLReader& reader)
{
    core::TRVec vertex;
    vertex.X = core::Length{static_cast<core::Length::type>(std::round(reader.readF()))};
    vertex.Y = core::Length{static_cast<core::Length::type>(std::round(reader.readF()))};
    vertex.Z = core::Length{static_cast<core::Length::type>(std::round(reader.readF()))};
    return vertex;
}
} // namespace io
} // namespace file
} // namespace loader
