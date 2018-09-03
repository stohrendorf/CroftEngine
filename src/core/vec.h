#pragma once

#include "gameplay.h"

#include <gsl/gsl>

#include <sol.hpp>

namespace loader
{
struct Room;
}

namespace core
{
struct TRVec
{
    using Scalar = int32_t;

    Scalar X = 0, Y = 0, Z = 0;

    TRVec() = default;

    TRVec(const TRVec&) = default;

    explicit TRVec(const glm::vec3& v)
            : X{gsl::narrow_cast<Scalar>( v.x )}
            , Y{-gsl::narrow_cast<Scalar>( v.y )}
            , Z{-gsl::narrow_cast<Scalar>( v.z )}
    {
    }

    explicit TRVec(const glm::ivec3& v)
            : X{v.x}
            , Y{-v.y}
            , Z{-v.z}
    {
    }

    TRVec(Scalar x, Scalar y, Scalar z)
            : X{x}
            , Y{y}
            , Z{z}
    {
    }

    TRVec operator-(const TRVec& rhs) const noexcept
    {
        return {X - rhs.X, Y - rhs.Y, Z - rhs.Z};
    }

    TRVec& operator-=(const TRVec& rhs) noexcept
    {
        X -= rhs.X;
        Y -= rhs.Y;
        Z -= rhs.Z;
        return *this;
    }

    TRVec operator/(int n) const noexcept
    {
        return {X / n, Y / n, Z / n};
    }

    TRVec& operator/=(int n) noexcept
    {
        X /= n;
        Y /= n;
        Z /= n;
        return *this;
    }

    TRVec operator+(const TRVec& rhs) const noexcept
    {
        return {X + rhs.X, Y + rhs.Y, Z + rhs.Z};
    }

    TRVec& operator+=(const TRVec& rhs) noexcept
    {
        X += rhs.X;
        Y += rhs.Y;
        Z += rhs.Z;
        return *this;
    }

    TRVec& operator=(const TRVec&) = default;

    glm::vec3 toRenderSystem() const noexcept
    {
        return {
                gsl::narrow_cast<glm::float_t>( X ),
                -gsl::narrow_cast<glm::float_t>( Y ),
                -gsl::narrow_cast<glm::float_t>( Z )
        };
    }

    int distanceTo(const TRVec& rhs) const
    {
        const auto dx = gsl::narrow<glm::float_t>( X - rhs.X );
        const auto dy = gsl::narrow<glm::float_t>( Y - rhs.Y );
        const auto dz = gsl::narrow<glm::float_t>( Z - rhs.Z );
        return static_cast<Scalar>(glm::sqrt( dx * dx + dy * dy + dz * dz ));
    }

    static sol::usertype<TRVec>& userType()
    {
        static sol::usertype<TRVec> userType(
                sol::meta_function::construct, sol::no_constructor,
                "x", &TRVec::X,
                "y", &TRVec::Y,
                "z", &TRVec::Z
        );

        return userType;
    }
};


struct RoomBoundPosition
{
    gsl::not_null<const loader::Room*> room;

    TRVec position;

    explicit RoomBoundPosition(const gsl::not_null<const loader::Room*>& r, const TRVec& pos = {})
            : room{r}
            , position{pos}
    {
    }
};
}
