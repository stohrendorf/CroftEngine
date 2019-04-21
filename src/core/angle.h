#pragma once

#include "core/units.h"

#include "gsl-lite.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <boost/optional.hpp>
#include <sol.hpp>
#include <yaml-cpp/yaml.h>

#include <cmath>

namespace core
{

constexpr int32_t FullRotation = 1 << 16;
constexpr int32_t AngleStorageScale = 1 << 16;


QS_DECLARE_QUANTITY( Angle, int32_t, "au" );

constexpr Angle auToAngle(int16_t value) noexcept;

constexpr Angle operator "" _deg(unsigned long long value) noexcept;

constexpr Angle operator "" _deg(long double value) noexcept;

inline Angle angleFromRad(const float r)
{
    return Angle{gsl::narrow_cast<Angle::type>( r / 2 / glm::pi<float>() * FullRotation * AngleStorageScale )};
}

inline Angle angleFromAtan(const float dx, const float dz)
{
    return angleFromRad( std::atan2( dx, dz ) );
}

inline Angle angleFromDegrees(const float value)
{
    return Angle{gsl::narrow_cast<Angle::type>( std::lround( value / 360 * FullRotation * AngleStorageScale ) )};
}

inline Angle angleFromAtan(const core::Length dx, const core::Length dz)
{
    return angleFromRad( std::atan2( dx.get_as<float>(), dz.get_as<float>() ) );
}

constexpr float toDegrees(const Angle& a) noexcept
{
    return a.get_as<float>() / AngleStorageScale * 360 / FullRotation;
}

inline float toRad(const Angle& a) noexcept
{
    return a.get_as<float>() / AngleStorageScale * glm::pi<float>() * 2 / FullRotation;
}

inline float sin(const Angle& a) noexcept
{
    return glm::sin( toRad( a ) );
}

inline float cos(const Angle& a) noexcept
{
    return glm::cos( toRad( a ) );
}

inline Angle abs(const Angle& a)
{
    return Angle{glm::abs( a.get() )};
}

enum class Axis
{
    PosZ,
    PosX,
    NegZ,
    NegX
};

inline boost::optional<Axis> axisFromAngle(const Angle& angle, const Angle& margin)
{
    Expects( margin >= 0_deg && margin <= 45_deg );
    if( angle <= +0_deg + margin && angle >= +0_deg - margin )
        return Axis::PosZ;
    if( angle <= +90_deg + margin && angle >= +90_deg - margin )
        return Axis::PosX;
    if( angle <= -90_deg + margin && angle >= -90_deg - margin )
        return Axis::NegX;
    if( angle >= 180_deg - margin || angle <= -180_deg + margin )
        return Axis::NegZ;

    return {};
}

inline Angle alignRotation(const Axis& axis)
{
    switch( axis )
    {
        case Axis::PosZ:
            return 0_deg;
        case Axis::PosX:
            return 90_deg;
        case Axis::NegZ:
            return -180_deg;
        case Axis::NegX:
            return -90_deg;
        default:
            return 0_deg;
    }
}

inline boost::optional<Angle> alignRotation(const Angle& angle, const Angle& margin)
{
    auto axis = axisFromAngle( angle, margin );
    if( !axis )
        return {};

    return alignRotation( *axis );
}


class TRRotation final
{
public:
    Angle X;

    Angle Y;

    Angle Z;

    TRRotation() = default;

    TRRotation(const Angle& x, const Angle& y, const Angle& z)
            : X{x}
            , Y{y}
            , Z{z}
    {
    }

    glm::vec3 toDegrees() const
    {
        return {
                core::toDegrees( X ),
                core::toDegrees( Y ),
                core::toDegrees( Z )
        };
    }

    glm::vec3 toRenderSystem() const
    {
        return {
                toRad( X ),
                -toRad( Y ),
                -toRad( Z )
        };
    }

    TRRotation operator-(const TRRotation& rhs) const
    {
        return {X - rhs.X, Y - rhs.Y, Z - rhs.Z};
    }

    glm::mat4 toMatrix() const
    {
        return glm::yawPitchRoll( -toRad( Y ), toRad( X ), -toRad( Z ) );
    }

    TRRotation operator-() const
    {
        return TRRotation{-X, -Y, -Z};
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
        X = n["x"].as<core::Angle>();
        Y = n["y"].as<core::Angle>();
        Z = n["z"].as<core::Angle>();
    }
};


inline glm::mat4 fromPackedAngles(uint32_t angleData)
{
    const auto getAngle = [angleData](const uint8_t n) -> Angle {
        BOOST_ASSERT( n < 3 );
        return core::auToAngle( ((angleData >> (10 * n)) & 0x3ff) * 64 );
    };

    TRRotation r{getAngle( 2 ), getAngle( 1 ), getAngle( 0 )};

    return r.toMatrix();
}


struct TRRotationXY
{
    Angle X;

    Angle Y;

    glm::mat4 toMatrix() const
    {
        return glm::yawPitchRoll( -toRad( Y ), toRad( X ), 0.0f );
    }

    YAML::Node save() const
    {
        YAML::Node n;
        n["x"] = X;
        n["y"] = Y;
        return n;
    }

    void load(const YAML::Node& n)
    {
        X = n["x"].as<core::Angle>();
        Y = n["y"].as<core::Angle>();
    }
};


inline TRRotationXY getVectorAngles(const core::Length& dx, const core::Length& dy, const core::Length& dz)
{
    const auto y = angleFromAtan( dx, dz );
    const auto dxz = sqrt( dx * dx + dz * dz );
    auto x = angleFromAtan( dy, dxz );
    if( (dy < 0_len) == std::signbit( toRad( x ) ) )
        x = -x;

    return TRRotationXY{x, y};
}

constexpr Angle auToAngle(int16_t value) noexcept
{
    return Angle{static_cast<Angle::type>(value) * AngleStorageScale};
}

constexpr Angle operator "" _au(const unsigned long long value) noexcept
{
    return auToAngle( static_cast<int16_t>(value) );
}

constexpr Angle operator "" _deg(const unsigned long long value) noexcept
{
    return Angle{static_cast<Angle::type>(value * FullRotation / 360 * AngleStorageScale)};
}

constexpr Angle operator "" _deg(const long double value) noexcept
{
    return Angle{static_cast<Angle::type>(value * FullRotation / 360 * AngleStorageScale)};
}
}

using core::operator ""_au;
using core::operator ""_deg;