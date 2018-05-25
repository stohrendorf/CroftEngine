#pragma once

#include "coordinates.h"

namespace core
{
struct BoundingBox final
{
    explicit BoundingBox() = default;

    explicit BoundingBox(const TRCoordinates& min_, const TRCoordinates& max_)
            : min{min_}
            , max{max_}
    {
    }

    bool isValid() const noexcept
    {
        return min.X <= max.X && min.Y <= max.Y && min.Z <= max.Z;
    }

    void makeValid() noexcept
    {
        if( min.X > max.X )
            std::swap( min.X, max.X );
        if( min.Y > max.Y )
            std::swap( min.Y, max.Y );
        if( min.Z > max.Z )
            std::swap( min.Z, max.Z );
    }

    bool intersects(const BoundingBox& box) const noexcept
    {
        return !(min.X > box.max.X || max.X < box.min.X
                 || min.Y > box.max.Y || max.Y < box.min.Y
                 || min.Z > box.max.Z || max.Z < box.min.Z);
    }

    bool contains(const TRCoordinates& v) const noexcept
    {
        return v.X >= min.X && v.X <= max.X
               && v.Y >= min.Y && v.Y <= max.Y
               && v.Z >= min.Z && v.Z <= max.Z;
    }

    TRCoordinates min{0, 0, 0};

    TRCoordinates max{0, 0, 0};
};
}
