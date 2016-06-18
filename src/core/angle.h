#pragma once

#include <cstdint>

#include <irrlicht.h>

#include <gsl.h>

namespace core
{
    namespace detail
    {
        struct UnsignedRawAngle;
    }

    class Angle final
    {
        friend struct detail::UnsignedRawAngle;
        friend Angle degToAngle(float);

        int32_t m_value;
        static const uint32_t Scale = 1U<<16;

        struct RawTag{};
        constexpr explicit Angle(int32_t val, const RawTag&) noexcept
            : m_value{ val }
        {
        }

    public:
        [[implicit]]
        constexpr Angle() noexcept
            : m_value{ 0 }
        {
        }

        explicit Angle(int16_t val) noexcept
            : m_value{ gsl::narrow_cast<int32_t>(val * Scale) }
        {
        }

        constexpr Angle(const Angle&) = default;

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
            return Angle{ gsl::narrow_cast<int32_t>(m_value - rhs.m_value), RawTag() };
        }

        Angle& operator-=(const Angle& rhs) noexcept
        {
            m_value -= rhs.m_value;
            return *this;
        }

        Angle operator+(const Angle& rhs) const noexcept
        {
            return Angle{ gsl::narrow_cast<int32_t>(m_value + rhs.m_value), RawTag() };
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

    inline Angle degToAngle(float val)
    {
        return Angle{ gsl::narrow_cast<int32_t>(std::lround(val * Angle::Scale)), Angle::RawTag() };
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
                : value{ gsl::narrow<uint32_t>(val * Angle::Scale) }
            {
                Expects(value <= 32768U * Angle::Scale);
            }

            explicit UnsignedRawAngle(long double val)
                : value{ gsl::narrow<uint32_t>(std::llround(val * Angle::Scale)) }
            {
                Expects(value <= 32768U * Angle::Scale);
            }

            Angle operator-() const
            {
                return Angle{ -gsl::narrow_cast<int32_t>(value), Angle::RawTag() };
            }

            Angle operator+() const
            {
                return static_cast<Angle>(*this);
            }

            operator Angle() const
            {
                return Angle{ gsl::narrow_cast<int32_t>(value), Angle::RawTag() };
            }
        };
    }
}

inline core::detail::UnsignedRawAngle operator"" _au(unsigned long long v) noexcept
{
    Expects(v <= 32768);
    return core::detail::UnsignedRawAngle{ v };
}

inline core::detail::UnsignedRawAngle operator"" _deg(unsigned long long v) noexcept
{
    Expects(v <= 180);
    return core::detail::UnsignedRawAngle{ v * 65536 / 360 };
}

inline core::detail::UnsignedRawAngle operator"" _deg(long double v) noexcept
{
    Expects(v <= 180);
    return core::detail::UnsignedRawAngle{ v * 65536 / 360 };
}
