#pragma once

#include "gameplay.h"

#include <gsl.h>

#include <cmath>


namespace loader
{
    struct Room;
}


namespace core
{
    struct TRCoordinates
    {
        long X = 0, Y = 0, Z = 0;

        TRCoordinates() = default;
        TRCoordinates(const TRCoordinates&) = default;


        explicit TRCoordinates(const gameplay::Vector3& v)
            : X(std::lround(v.x))
            , Y(-std::lround(v.y))
            , Z(-std::lround(v.z))
        {
        }


        TRCoordinates(long x, long y, long z)
            : X(x)
            , Y(y)
            , Z(z)
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


        gameplay::Vector3 toRenderSystem() const noexcept
        {
            return {gsl::narrow_cast<float>(X), -gsl::narrow_cast<float>(Y), -gsl::narrow_cast<float>(Z)};
        }
    };


    struct ExactTRCoordinates
    {
        float X = 0, Y = 0, Z = 0;

        ExactTRCoordinates() = default;
        ExactTRCoordinates(const ExactTRCoordinates&) = default;


        explicit ExactTRCoordinates(const gameplay::Vector3& v)
            : X(v.x)
            , Y(-v.y)
            , Z(-v.z)
        {
        }


        explicit ExactTRCoordinates(const TRCoordinates& v)
            : X(float(v.X))
            , Y(float(v.Y))
            , Z(float(v.Z))
        {
        }


        ExactTRCoordinates(float x, float y, float z)
            : X(x)
            , Y(y)
            , Z(z)
        {
        }


        ExactTRCoordinates operator-(const ExactTRCoordinates& rhs) const noexcept
        {
            return {X - rhs.X, Y - rhs.Y, Z - rhs.Z};
        }


        ExactTRCoordinates& operator-=(const ExactTRCoordinates& rhs) noexcept
        {
            X -= rhs.X;
            Y -= rhs.Y;
            Z -= rhs.Z;
            return *this;
        }


        ExactTRCoordinates operator+(const ExactTRCoordinates& rhs) const noexcept
        {
            return {X + rhs.X, Y + rhs.Y, Z + rhs.Z};
        }


        ExactTRCoordinates& operator+=(const ExactTRCoordinates& rhs) noexcept
        {
            X += rhs.X;
            Y += rhs.Y;
            Z += rhs.Z;
            return *this;
        }


        ExactTRCoordinates operator*(float f) const noexcept
        {
            return {X * f, Y * f, Z * f};
        }


        ExactTRCoordinates& operator*=(float f) noexcept
        {
            X *= f;
            Y *= f;
            Z *= f;
            return *this;
        }


        ExactTRCoordinates operator/(float f) const noexcept
        {
            return {X / f, Y / f, Z / f};
        }


        ExactTRCoordinates& operator/=(float f) noexcept
        {
            X /= f;
            Y /= f;
            Z /= f;
            return *this;
        }


        ExactTRCoordinates& operator=(const ExactTRCoordinates& rhs)
        {
            X = rhs.X;
            Y = rhs.Y;
            Z = rhs.Z;
            return *this;
        }


        gameplay::Vector3 toRenderSystem() const noexcept
        {
            return {X, -Y, -Z};
        }


        TRCoordinates toInexact() const noexcept
        {
            return {std::lround(X), std::lround(Y), std::lround(Z)};
        }


        float distanceTo(const ExactTRCoordinates& b) const
        {
            auto d = *this - b;
            return std::sqrt(d.X * d.X + d.Y * d.Y + d.Z * d.Z);
        }
    };


    struct RoomBoundPosition
    {
        gsl::not_null<const loader::Room*> room;
        ExactTRCoordinates position;


        explicit RoomBoundPosition(const gsl::not_null<const loader::Room*>& r, const ExactTRCoordinates& pos = {})
            : room(r)
            , position(pos)
        {
        }
    };


    struct RoomBoundIntPosition
    {
        gsl::not_null<const loader::Room*> room;
        TRCoordinates position;


        explicit RoomBoundIntPosition(const gsl::not_null<const loader::Room*>& r, const TRCoordinates& pos = {})
            : room(r)
            , position(pos)
        {
        }
    };
}
