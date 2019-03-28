#pragma once

#include "units.h"

#include "gsl-lite.hpp"

#include <sol.hpp>
#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

namespace loader
{
namespace file
{
struct Room;
}
}

namespace core
{
struct TRVec
{
    Length X = 0_len, Y = 0_len, Z = 0_len;

    constexpr TRVec() noexcept = default;

    constexpr TRVec(const TRVec&) noexcept = default;

    constexpr TRVec(TRVec&&) noexcept = default;

    explicit constexpr TRVec(const glm::vec3& v) noexcept
            : X{gsl::narrow_cast<Length::type>( v.x )}
            , Y{-gsl::narrow_cast<Length::type>( v.y )}
            , Z{-gsl::narrow_cast<Length::type>( v.z )}
    {
    }

    constexpr explicit TRVec(const glm::ivec3& v) noexcept
            : X{gsl::narrow_cast<Length::type>( v.x )}
            , Y{-gsl::narrow_cast<Length::type>( v.y )}
            , Z{-gsl::narrow_cast<Length::type>( v.z )}
    {
    }

    constexpr TRVec(const Length x, const Length y, const Length z) noexcept
            : X{x}
            , Y{y}
            , Z{z}
    {
    }

    ~TRVec() noexcept = default;

    constexpr TRVec& operator=(const TRVec&) noexcept = default;

    constexpr TRVec& operator=(TRVec&&) noexcept = default;

    constexpr TRVec operator-(const TRVec& rhs) const noexcept
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

    constexpr TRVec operator/(const core::Length::type n) const noexcept
    {
        return {X / n, Y / n, Z / n};
    }

    TRVec& operator/=(const int n) noexcept
    {
        X /= n;
        Y /= n;
        Z /= n;
        return *this;
    }

    constexpr TRVec operator+(const TRVec& rhs) const noexcept
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

    glm::vec3 toRenderSystem() const noexcept
    {
        return {
                gsl::narrow_cast<float>( X.get() ),
                -gsl::narrow_cast<float>( Y.get() ),
                -gsl::narrow_cast<float>( Z.get() )
        };
    }

    Length distanceTo(const TRVec& rhs) const
    {
        const auto dx = gsl::narrow<float>( (X - rhs.X).get() );
        const auto dy = gsl::narrow<float>( (Y - rhs.Y).get() );
        const auto dz = gsl::narrow<float>( (Z - rhs.Z).get() );
        return Length{static_cast<Length::type>( glm::sqrt( dx * dx + dy * dy + dz * dz ) )};
    }

    static sol::usertype<TRVec>& userType()
    {
        static sol::usertype<TRVec> userType(
                sol::constructors<TRVec(), TRVec(Length, Length, Length)>(),
                "x", &TRVec::X,
                "y", &TRVec::Y,
                "z", &TRVec::Z
        );

        return userType;
    }

    YAML::Node save() const
    {
        YAML::Node n;
        n.SetStyle( YAML::EmitterStyle::Flow );
        n["x"] = X;
        n["y"] = Y;
        n["z"] = Z;
        return n;
    }

    void load(const YAML::Node& n)
    {
        X = n["x"].as<Length>();
        Y = n["y"].as<Length>();
        Z = n["z"].as<Length>();
    }

    core::Length length() const
    {
        return sqrt( X * X + Y * Y + Z * Z );
    }
};


struct RoomBoundPosition
{
    gsl::not_null<const loader::file::Room*> room;

    TRVec position;

    explicit RoomBoundPosition(const gsl::not_null<const loader::file::Room*>& r, const TRVec& pos = {})
            : room{r}
            , position{pos}
    {
    }
};
}
