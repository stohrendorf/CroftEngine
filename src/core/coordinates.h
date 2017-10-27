#pragma once

#include "gameplay.h"

#include <gsl/gsl>

#include <cmath>


namespace loader
{
    struct Room;
}


namespace core
{
    struct TRCoordinates
    {
        int32_t X = 0, Y = 0, Z = 0;

        TRCoordinates() = default;

        TRCoordinates(const TRCoordinates&) = default;


        explicit TRCoordinates(const glm::vec3& v)
            : X{gsl::narrow_cast<int>(v.x)}
            , Y{-gsl::narrow_cast<int>(v.y)}
            , Z{-gsl::narrow_cast<int>(v.z)}
        {
        }


        explicit TRCoordinates(const glm::ivec3& v)
            : X{v.x}
            , Y{-v.y}
            , Z{-v.z}
        {
        }


        TRCoordinates(int x, int y, int z)
            : X{x}
            , Y{y}
            , Z{z}
        {
        }


        TRCoordinates operator-(const TRCoordinates& rhs) const noexcept
        {
            return {X - rhs.X, Y - rhs.Y, Z - rhs.Z};
        }


        TRCoordinates& operator-=(const TRCoordinates& rhs) noexcept
        {
            X -= rhs.X;
            Y -= rhs.Y;
            Z -= rhs.Z;
            return *this;
        }


        TRCoordinates operator/(int n) const noexcept
        {
            return {X / n, Y / n, Z / n};
        }


        TRCoordinates& operator/=(int n) noexcept
        {
            X /= n;
            Y /= n;
            Z /= n;
            return *this;
        }


        TRCoordinates operator+(const TRCoordinates& rhs) const noexcept
        {
            return {X + rhs.X, Y + rhs.Y, Z + rhs.Z};
        }


        TRCoordinates& operator+=(const TRCoordinates& rhs) noexcept
        {
            X += rhs.X;
            Y += rhs.Y;
            Z += rhs.Z;
            return *this;
        }


        TRCoordinates& operator=(const TRCoordinates&) = default;


        glm::vec3 toRenderSystem() const noexcept
        {
            return {gsl::narrow_cast<float>(X), -gsl::narrow_cast<float>(Y), -gsl::narrow_cast<float>(Z)};
        }


        int distanceTo(const TRCoordinates& rhs) const
        {
            const auto dx = gsl::narrow<float>(X - rhs.X);
            const auto dy = gsl::narrow<float>(Y - rhs.Y);
            const auto dz = gsl::narrow<float>(Z - rhs.Z);
            return static_cast<int>(sqrtf(dx * dx + dy * dy + dz * dz));
        }
    };


    struct RoomBoundPosition
    {
        gsl::not_null<const loader::Room*> room;

        TRCoordinates position;


        explicit RoomBoundPosition(const gsl::not_null<const loader::Room*>& r, const TRCoordinates& pos = {})
            : room{r}
            , position{pos}
        {
        }
    };
}
