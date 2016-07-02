#pragma once

#include <cmath>

#include <irrlicht.h>

#include <gsl.h>

#include <boost/optional.hpp>

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
        [[implicit]]
        constexpr Angle() noexcept
            : m_value{0}
        {
        }

        explicit Angle(int16_t val) noexcept
            : m_value{gsl::narrow_cast<int32_t>(val * Scale)}
        {
        }

        constexpr Angle(const Angle&) = default;

        static Angle fromRad(float r)
        {
            return Angle{gsl::narrow_cast<int32_t>(r / 2 / irr::core::PI * 65536 * Scale), RawTag()};
        }

        static Angle fromDegrees(float val)
        {
            return Angle{gsl::narrow_cast<int32_t>(std::lround(val * Scale)), RawTag()};
        }

        constexpr float toDegrees() const noexcept
        {
            return m_value * 360.0f / Scale / 65536;
        }

        float toRad() const noexcept
        {
            return m_value * irr::core::PI * 2 / Scale / 65536;
        }

        float sin() const noexcept
        {
            return std::sin(toRad());
        }

        float cos() const noexcept
        {
            return std::cos(toRad());
        }

        constexpr int16_t toAU() const noexcept
        {
            return m_value / Scale;
        }

        Angle operator-(const Angle& rhs) const noexcept
        {
            return Angle{gsl::narrow_cast<int32_t>(m_value - rhs.m_value), RawTag()};
        }

        Angle& operator-=(const Angle& rhs) noexcept
        {
            m_value -= rhs.m_value;
            return *this;
        }

        Angle operator+(const Angle& rhs) const noexcept
        {
            return Angle{gsl::narrow_cast<int32_t>(m_value + rhs.m_value), RawTag()};
        }

        Angle& operator+=(const Angle& rhs) noexcept
        {
            m_value += rhs.m_value;
            return *this;
        }

        Angle operator*(float v) const
        {
            return Angle{gsl::narrow_cast<int32_t>(std::lround(m_value * v)), RawTag()};
        }

        Angle& operator*=(float v)
        {
            m_value = gsl::narrow_cast<int32_t>(v);
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
                : value{gsl::narrow<uint32_t>(val * Angle::Scale)}
            {
                Expects(value <= 32768U * Angle::Scale);
            }

            explicit UnsignedRawAngle(long double val)
                : value{gsl::narrow<uint32_t>(std::llround(val * Angle::Scale))}
            {
                Expects(value <= 32768U * Angle::Scale);
            }

            Angle operator-() const
            {
                return Angle{-gsl::narrow_cast<int32_t>(value), Angle::RawTag()};
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
                return Angle{gsl::narrow_cast<int32_t>(value), Angle::RawTag()};
            }
        };

        inline UnsignedRawAngle operator"" _au(unsigned long long v) noexcept
        {
            Expects(v <= 32768);
            return UnsignedRawAngle{v};
        }

        inline UnsignedRawAngle operator"" _deg(unsigned long long v) noexcept
        {
            Expects(v <= 180);
            return UnsignedRawAngle{v * 65536 / 360};
        }

        inline UnsignedRawAngle operator"" _deg(long double v) noexcept
        {
            Expects(v <= 180);
            return UnsignedRawAngle{v * 65536 / 360};
        }
    }

    enum class Axis
    {
        PosZ,
        PosX,
        NegZ,
        NegX
    };

    using detail::operator "" _au;
    using detail::operator "" _deg;

    inline boost::optional<Axis> axisFromAngle(const core::Angle& angle, const core::Angle& margin)
    {
        Expects(margin >= 0_deg && margin <= 45_deg);
        if( angle <= 0_deg + margin && angle >= 0_deg - margin )
            return Axis::PosZ;
        if( angle <= 90_deg + margin && angle >= 90_deg - margin )
            return Axis::PosX;
        if( angle <= -90_deg + margin && angle >= -90_deg - margin )
            return Axis::NegX;
        if( angle >= 180_deg - margin || angle <= -180_deg + margin )
            return Axis::NegZ;

        return {};
    }

    inline core::Angle alignRotation(const Axis& axis)
    {
        switch(axis)
        {
            case Axis::PosZ: return core::Angle(0_deg);
            case Axis::PosX: return core::Angle(90_deg);
            case Axis::NegZ: return core::Angle(-180_deg);
            case Axis::NegX: return core::Angle(-90_deg);
            default: return 0_deg;
        }
    }

    inline boost::optional<core::Angle> alignRotation(const core::Angle& angle, const core::Angle& margin)
    {
        auto axis = axisFromAngle(angle, margin);
        if( !axis )
            return {};

        return alignRotation(*axis);
    }
}

using core::detail::operator "" _au;
using core::detail::operator "" _deg;
