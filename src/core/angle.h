#pragma once

#include <cmath>

#include <gsl/gsl>
#include <glm/gtx/euler_angles.hpp>

#include <boost/optional.hpp>
#include <sol.hpp>
#include <yaml-cpp/yaml.h>

namespace core
{
namespace detail
{
struct UnsignedRawAngle;
}
}

inline core::detail::UnsignedRawAngle operator "" _au(unsigned long long value) noexcept;

inline core::detail::UnsignedRawAngle operator "" _deg(unsigned long long value) noexcept;

inline core::detail::UnsignedRawAngle operator "" _deg(long double value) noexcept;

namespace core
{
namespace detail
{
struct UnsignedRawAngle;
}

class Angle final
{
    friend struct detail::UnsignedRawAngle;

    int32_t m_value = 0;

    static const int32_t Scale = 1 << 16;

    struct RawTag
    {
    };

    constexpr explicit Angle(const int32_t val, const RawTag&) noexcept
            : m_value{val}
    {
    }

public:
    constexpr Angle() noexcept = default;

    explicit Angle(const int16_t value) noexcept
            : m_value{gsl::narrow_cast<int32_t>( value * Scale )}
    {
    }

    constexpr Angle(const Angle&) = default;

    constexpr Angle(Angle&&) = default;

    Angle& operator=(const Angle&) = default;

    Angle& operator=(Angle&&) = default;

    ~Angle() noexcept = default;

    static Angle fromRad(const float r)
    {
        return Angle{gsl::narrow_cast<int32_t>( r / 2 / glm::pi<float>() * 65536 * Scale ), RawTag()};
    }

    static Angle fromAtan(const float dx, const float dz)
    {
        return fromRad( std::atan2( dx, dz ) );
    }

    static Angle fromDegrees(const float value)
    {
        return Angle{gsl::narrow_cast<int32_t>( std::lround( value / 360 * 65536 * Scale ) ), RawTag()};
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
        return glm::sin( toRad() );
    }

    float cos() const noexcept
    {
        return glm::cos( toRad() );
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

    Angle operator*(const float value) const
    {
        return Angle{gsl::narrow_cast<int32_t>( std::lround( m_value * value ) ), RawTag()};
    }

    Angle& operator*=(const float value)
    {
        m_value = gsl::narrow_cast<int32_t>( m_value * value );
        return *this;
    }

    Angle operator/(const float value) const
    {
        return Angle{gsl::narrow_cast<int32_t>( std::lround( m_value / value ) ), RawTag()};
    }

    Angle& operator/=(const float value)
    {
        m_value = gsl::narrow_cast<int32_t>( m_value / value );
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
        return Angle{glm::abs( m_value ), RawTag{}};
    }

    static sol::usertype<Angle>& userType()
    {
        static sol::usertype<Angle> userType(
                sol::meta_function::construct, sol::no_constructor,
                "sin", &Angle::sin,
                "cos", &Angle::cos,
                "abs", &Angle::abs,
                "get_au", &Angle::toAU
        );

        return userType;
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
    const uint32_t value{};

    explicit UnsignedRawAngle(const unsigned long long value)
            : value{gsl::narrow<uint32_t>( value * Angle::Scale )}
    {
        Expects( value <= 32768U * Angle::Scale );
    }

    explicit UnsignedRawAngle(const long double value)
            : value{gsl::narrow<uint32_t>( std::lround( value * Angle::Scale ) )}
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

    // ReSharper disable once CppNonExplicitConversionOperator
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

    TRRotation operator-() const
    {
        return TRRotation{-X, -Y, -Z};
    }

    static sol::usertype<TRRotation>& userType()
    {
        static sol::usertype<TRRotation> userType(
                sol::meta_function::construct, sol::no_constructor,
                "x", &TRRotation::X,
                "y", &TRRotation::Y,
                "z", &TRRotation::Z
        );

        return userType;
    }

    YAML::Node save() const
    {
        YAML::Node n;
        n.SetStyle( YAML::EmitterStyle::Flow );
        n["x"] = X.toDegrees();
        n["y"] = Y.toDegrees();
        n["z"] = Z.toDegrees();
        return n;
    }

    void load(const YAML::Node& n)
    {
        if( !n["x"].IsScalar() )
            BOOST_THROW_EXCEPTION( std::domain_error( "TRRotation::X is not a scalar value" ) );
        if( !n["y"].IsScalar() )
            BOOST_THROW_EXCEPTION( std::domain_error( "TRRotation::Y is not a scalar value" ) );
        if( !n["z"].IsScalar() )
            BOOST_THROW_EXCEPTION( std::domain_error( "TRRotation::Z is not a scalar value" ) );

        X = Angle::fromDegrees( n["x"].as<float>() );
        Y = Angle::fromDegrees( n["y"].as<float>() );
        Z = Angle::fromDegrees( n["z"].as<float>() );
    }
};


inline glm::mat4 fromPackedAngles(uint32_t angleData)
{
    const auto getAngle = [angleData](const uint8_t n) -> Angle {
        BOOST_ASSERT( n < 3 );
        return Angle( static_cast<int16_t>(((angleData >> (10 * n)) & 0x3ff) * 64) );
    };

    TRRotation r{getAngle( 2 ), getAngle( 1 ), getAngle( 0 )};

    return r.toMatrix();
}

inline Angle abs(const Angle& angle)
{
    return angle.abs();
}


struct TRRotationXY
{
    Angle X;

    Angle Y;

    glm::mat4 toMatrix() const
    {
        return glm::yawPitchRoll( -Y.toRad(), X.toRad(), 0.0f );
    }

    YAML::Node save() const
    {
        YAML::Node n;
        n["x"] = X.toDegrees();
        n["y"] = Y.toDegrees();
        return n;
    }

    void load(const YAML::Node& n)
    {
        if( !n["x"].IsScalar() )
            BOOST_THROW_EXCEPTION( std::domain_error( "TRRotationXY::X is not a scalar value" ) );
        if( !n["y"].IsScalar() )
            BOOST_THROW_EXCEPTION( std::domain_error( "TRRotationXY::Y is not a scalar value" ) );

        X = Angle::fromDegrees( n["x"].as<float>() );
        Y = Angle::fromDegrees( n["y"].as<float>() );
    }
};


inline TRRotationXY getVectorAngles(const float dx, const float dy, const float dz)
{
    const auto y = Angle::fromAtan( dx, dz );
    const auto dxz = glm::sqrt( dz * dz + dx * dx );
    auto x = Angle::fromAtan( dy, dxz );
    if( std::signbit( dy ) == std::signbit( x.toRad() ) )
        x = -x;

    return TRRotationXY{x, y};
}
}

inline core::detail::UnsignedRawAngle operator "" _au(const unsigned long long value) noexcept
{
    Expects( value <= 32768 );
    return core::detail::UnsignedRawAngle{value};
}

inline core::detail::UnsignedRawAngle operator "" _deg(const unsigned long long value) noexcept
{
    Expects( value <= 180 );
    return core::detail::UnsignedRawAngle{value * 65536 / 360};
}

inline core::detail::UnsignedRawAngle operator "" _deg(const long double value) noexcept
{
    Expects( value <= 180 );
    return core::detail::UnsignedRawAngle{value * 65536 / 360};
}
