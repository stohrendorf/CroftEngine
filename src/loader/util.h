#pragma once

#include "core/vec.h"
#include "io/sdlreader.h"

namespace loader
{
namespace io
{
inline core::TRVec readCoordinates16(io::SDLReader& reader)
{
    core::TRVec vertex;
    vertex.X = reader.readI16();
    vertex.Y = reader.readI16();
    vertex.Z = reader.readI16();
    return vertex;
}

inline core::TRVec readCoordinates32(io::SDLReader& reader)
{
    core::TRVec vertex;
    vertex.X = reader.readI32();
    vertex.Y = reader.readI32();
    vertex.Z = reader.readI32();
    return vertex;
}

inline core::TRVec readCoordinatesF(io::SDLReader& reader)
{
    core::TRVec vertex;
    vertex.X = std::lround( reader.readF() );
    vertex.Y = std::lround( reader.readF() );
    vertex.Z = std::lround( reader.readF() );
    return vertex;
}
}
}
