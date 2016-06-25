#pragma once

#include "core/coordinates.h"
#include "io/sdlreader.h"

namespace loader
{
    namespace io
    {
        inline core::TRCoordinates readCoordinates16(io::SDLReader& reader)
        {
            core::TRCoordinates vertex;
            // read vertex and change coordinate system
            vertex.X = reader.readI16();
            vertex.Y = reader.readI16();
            vertex.Z = reader.readI16();
            return vertex;
        }

        inline core::TRCoordinates readCoordinates32(io::SDLReader& reader)
        {
            core::TRCoordinates vertex;
            // read vertex and change coordinate system
            vertex.X = reader.readI32();
            vertex.Y = reader.readI32();
            vertex.Z = reader.readI32();
            return vertex;
        }

        inline core::TRCoordinates readCoordinatesF(io::SDLReader& reader)
        {
            core::TRCoordinates vertex;
            vertex.X = std::lround(reader.readF());
            vertex.Y = std::lround(reader.readF());
            vertex.Z = std::lround(reader.readF());
            return vertex;
        }
    }
}
