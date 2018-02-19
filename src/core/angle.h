#pragma once

#include "gameplay.h"

#include <cmath>

#include <gsl/gsl>
#include <glm/gtx/euler_angles.hpp>

#include <boost/optional.hpp>
#include <scriptengine/lua/sol.hpp>

namespace core
{
namespace detail
{
struct UnsignedRawAngle;
}
}

inline core::detail::UnsignedRawAngle operator "" _au(unsigned long long v) noexcept;

inline core::detail::UnsignedRawAngle operator "" _deg(unsigned long long v) noexcept;

inline core::detail::UnsignedRawAngle operator "" _deg(long double v) noexcept;

namespace core
{
namespace detail
{
struct UnsignedRawAngle;
}

class Angle final
{
    friend struct detail::UnsignedRawAngle;

    int32_t m_value;
    static const int32_t Scale = 1 << 16;

    struct RawTag
    {
    };

    constexpr explicit Angle(int32_t val, const RawTag&) noexcept
            : m_value{val}
    {
    }

public:
    constexpr Angle() noexcept
            : m_value{0}
    {
    }

    explicit Angle(int16_t val) noexcept
            : m_value{gsl::narrow_cast<int32_t>( val * Scale )}
    {
    }

    constexpr Angle(const Angle&) = default;

    static Angle fromRad(float r)
    {
        return Angle{gsl::narrow_cast<int32_t>( r / 2 / glm::pi<float>() * 65536 * Scale ), RawTag()};
    }

    static Angle fromAtan(float dx, float dz)
    {
        return fromRad( std::atan2( dx, dz ) );
    }

    static Angle fromDegrees(float val)
    {
        auto result = Angle{gsl::narrow_cast<int32_t>( std::lround( val * Scale ) ), RawTag()};
        return result;
    }

    constexpr float toDegrees() const noexcept
    {
        return m_value * 360.0f / Scale / 65536;
    }

    float toRad() const noexcept
    {
        return m_value * glm::pi<float>() * 2 / Scale / 65536;
    }

    float sin() const noexcept
    {
        return std::sin( toRad() );
    }

    float cos() const noexcept
    {
        return std::cos( toRad() );
    }

    constexpr int16_t toAU() const noexcept
    {
        return static_cast<int16_t>(m_value / Scale);
    }

    Angle operator-(const Angle& rhs) const noexcept
    {
        return Angle{gsl::narrow_cast<int32_t>( m_value - rhs.m_value ), RawTag()};
    }

    Angle& operator-=(const Angle& rhs) noexcept
    {
        m_value -= rhs.m_value;
        return *this;
    }

    Angle operator+(const Angle& rhs) const noexcept
    {
        return Angle{gsl::narrow_cast<int32_t>( m_value + rhs.m_value ), RawTag()};
    }

    Angle& operator+=(const Angle& rhs) noexcept
    {
        m_value += rhs.m_value;
        return *this;
    }

    Angle operator*(float v) const
    {
        auto result = Angle{gsl::narrow_cast<int32_t>( std::lround( m_value * v ) ), RawTag()};
        return result;
    }

    Angle& operator*=(float v)
    {
        m_value = gsl::narrow_cast<int32_t>( m_value * v );
        return *this;
    }

    Angle operator/(float v) const
    {
        auto result = Angle{gsl::narrow_cast<int32_t>( std::lround( m_value / v ) ), RawTag()};
        return result;
    }

    Angle& operator/=(float v)
    {
        m_value = gsl::narrow_cast<int32_t>( m_value / v );
        return *this;
    }

    Angle operator-() const
    {
        return Angle{-m_value, RawTag()};
    }

    constexpr bool operator==(const Angle& rhs) const noexcept
    {
        return m_value == rhs.m_value;
    }

    constexpr bool operator<(const Angle& rhs) const noexcept
    {
        return m_value < rhs.m_value;
    }

    Angle abs() const noexcept
    {
        return Angle{std::abs( m_value ), RawTag{}};
    }

    static sol::usertype<Angle> userType()
    {
        return sol::usertype<Angle>(
                sol::meta_function::construct, sol::no_constructor,
                "sin", &Angle::sin,
                "cos", &Angle::cos,
                "abs", &Angle::abs,
                "get_au", &Angle::toAU
        );
    }
};


constexpr bool operator>(const Angle& a, const Angle& b) noexcept
{
    return b < a;
}

constexpr bool operator<=(const Angle& a, const Angle& b) noexcept
{
    return a == b || a < b;
}

constexpr bool operator>=(const Angle& a, const Angle& b) noexcept
{
    return a == b || a > b;
}

constexpr bool operator!=(const Angle& a, const Angle& b) noexcept
{
    return !(a == b);
}

namespace detail
{
/**
 * @brief A simple helper to provide negation of unsigned values created by operator""_au
 */
struct UnsignedRawAngle final
{
    const uint32_t value;

    explicit UnsignedRawAngle(unsigned long long val)
            : value{gsl::narrow<uint32_t>( val * Angle::Scale )}
    {
        Expects( value <= 32768U * Angle::Scale );
    }

    explicit UnsignedRawAngle(long double val)
            : value{gsl::narrow<uint32_t>( std::lround( val * Angle::Scale ) )}
    {
        Expects( value <= 32768U * Angle::Scale );
    }

    Angle operator-() const
    {
        return Angle{-gsl::narrow_cast<int32_t>( value ), Angle::RawTag()};
    }

    Angle operator+() const
    {
        return static_cast<Angle>(*this);
    }

    Angle operator-(const Angle& rhs) const
    {
        return static_cast<Angle>(*this) - rhs;
    }

    Angle operator+(const Angle& rhs) const
    {
        return static_cast<Angle>(*this) + rhs;
    }

    operator Angle() const
    {
        return Angle{gsl::narrow_cast<int32_t>( value ), Angle::RawTag()};
    }
};
}

enum class Axis
{
    PosZ,
    PosX,
    NegZ,
    NegX
};
}

namespace core
{
inline boost::optional<Axis> axisFromAngle(const core::Angle& angle, const core::Angle& margin)
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

inline core::Angle alignRotation(const Axis& axis)
{
    switch( axis )
    {
        case Axis::PosZ:
            return core::Angle( 0_deg );
        case Axis::PosX:
            return core::Angle( 90_deg );
        case Axis::NegZ:
            return core::Angle( -180_deg );
        case Axis::NegX:
            return core::Angle( -90_deg );
        default:
            return 0_deg;
    }
}

inline boost::optional<core::Angle> alignRotation(const core::Angle& angle, const core::Angle& margin)
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
                X.toDegrees(),
                Y.toDegrees(),
                Z.toDegrees()
        };
    }

    glm::vec3 toRenderSystem() const
    {
        return {
                X.toRad(),
                -Y.toRad(),
                -Z.toRad()
        };
    }

    TRRotation operator-(const TRRotation& rhs) const
    {
        return {X - rhs.X, Y - rhs.Y, Z - rhs.Z};
    }

    glm::mat4 toMatrix() const
    {
        return glm::yawPitchRoll( -Y.toRad(), X.toRad(), -Z.toRad() );
    }

    static sol::usertype<TRRotation> userType()
    {
        return sol::usertype<TRRotation>(
                sol::meta_function::construct, sol::no_constructor,
                "x", &TRRotation::X,
                "y", &TRRotation::Y,
                "z", &TRRotation::Z
        );
    }
};


inline glm::mat4 fromPackedAngles(uint32_t angleData)
{
    auto getAngle = [angleData](uint8_t n) -> Angle {
        BOOST_ASSERT( n < 3 );
        return Angle( static_cast<int16_t>(((angleData >> 10 * n) & 0x3ff) * 64) );
    };

    TRRotation r{getAngle( 2 ), getAngle( 1 ), getAngle( 0 )};

    return r.toMatrix();
}

inline Angle abs(const Angle& v)
{
    return v.abs();
}

struct TRRotationXY {
    core::Angle X;
    core::Angle Y;
};
inline TRRotationXY anglesFromPosition(float dx, float dy, float dz)
{
    const Angle y = Angle::fromAtan(dx, dz);
    const auto dxz = std::sqrtf(dz*dz + dx*dx);
    Angle x = Angle::fromAtan(dy, dxz);
    if(dy > 0 && x > 0_deg || dy < 0 && x < 0_deg)
        x = -x;

    return TRRotationXY{x, y};
}
}

inline core::detail::UnsignedRawAngle operator "" _au(unsigned long long v) noexcept
{
    Expects( v <= 32768 );
    return core::detail::UnsignedRawAngle{v};
}

inline core::detail::UnsignedRawAngle operator "" _deg(unsigned long long v) noexcept
{
    Expects( v <= 180 );
    return core::detail::UnsignedRawAngle{v * 65536 / 360};
}

inline core::detail::UnsignedRawAngle operator "" _deg(long double v) noexcept
{
    Expects( v <= 180 );
    return core::detail::UnsignedRawAngle{v * 65536 / 360};
}
