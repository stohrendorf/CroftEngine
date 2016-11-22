#pragma once

#include <type_traits>

#include "magic.h"

namespace core
{
    template <typename T>
    class InterpolatedValue
    {
        T m_value{};

    public:
        using Type = T;
        using SelfType = InterpolatedValue<T>;

        template <typename... Args>
        explicit constexpr InterpolatedValue(Args&&... args)
            : m_value{std::forward<Args>(args)...}
        {
        }

        const T& getCurrentValue() const noexcept
        {
            return m_value;
        }

        explicit operator const T&() const noexcept
        {
            return m_value;
        }

        explicit operator T&() noexcept
        {
            return m_value;
        }

        SelfType& add(const T& v, const std::chrono::microseconds& ms)
        {
            m_value += scale(v, ms);
            return *this;
        }

        SelfType& add(const SelfType& v, const std::chrono::microseconds& ms)
        {
            m_value += v.getScaled(ms);
            return *this;
        }

        SelfType& sub(const T& v, const std::chrono::microseconds& ms)
        {
            m_value -= scale(v, ms);
            return *this;
        }

        SelfType& sub(const SelfType& v, const std::chrono::microseconds& ms)
        {
            m_value -= v.getScaled(ms);
            return *this;
        }

        T getScaled(const std::chrono::microseconds& ms) const noexcept
        {
            return scale(m_value, ms);
        }

        SelfType& limitMin(const T& v)
        {
            if( m_value < v )
                m_value = v;
            return *this;
        }

        SelfType& limitMax(const T& v)
        {
            if( m_value > v )
                m_value = v;
            return *this;
        }

        SelfType& limit(const T& min, const T& max)
        {
            return limitMin(min).limitMax(max);
        }

        static constexpr T scale(const T& v, const std::chrono::microseconds& ms)
        {
            // Use a pre-calculated factor to avoid overflows when multiplying.
            return v * (static_cast<float>(ms.count()) / core::FrameTime.count());
        }

        template <typename U>
        SelfType operator*(const U& f) const
        {
            return SelfType{m_value * f};
        }

        template <typename U>
        SelfType& operator*=(const U& f)
        {
            m_value *= f;
            return *this;
        }

        template <typename U>
        SelfType operator/(const U& f) const
        {
            return SelfType{m_value / f};
        }

        template <typename U>
        SelfType& operator/=(const U& f)
        {
            m_value /= f;
            return *this;
        }

        template <typename U>
        bool operator<(const U& v) const
        {
            return m_value < v;
        }

        bool operator<(const SelfType& v) const
        {
            return m_value < v.m_value;
        }

        template <typename U>
        bool operator>(const U& v) const
        {
            return m_value > v;
        }

        bool operator>(const SelfType& v) const
        {
            return m_value > v.m_value;
        }

        template <typename U>
        bool operator==(const U& v) const
        {
            return m_value == v;
        }

        bool operator==(const SelfType& v) const
        {
            return m_value == v.m_value;
        }

        SelfType& operator-=(const T& v)
        {
            m_value -= v;
            return *this;
        }

        SelfType operator-(const T& v) const
        {
            return SelfType{m_value - v};
        }

        SelfType& operator+=(const T& v)
        {
            m_value += v;
            return *this;
        }

        SelfType operator+(const T& v) const
        {
            return SelfType{m_value + v};
        }

        SelfType& operator-=(const SelfType& v)
        {
            m_value -= v.m_value;
            return *this;
        }

        SelfType operator-(const SelfType& v) const
        {
            return SelfType{m_value - v.m_value};
        }

        SelfType& operator+=(const SelfType& v)
        {
            m_value += v.m_value;
            return *this;
        }

        SelfType operator+(const SelfType& v) const
        {
            return SelfType{m_value + v.m_value};
        }

        SelfType& operator=(const SelfType&) = default;

        SelfType& operator=(const T& v)
        {
            m_value = v;
            return *this;
        }
    };

    template <typename T, typename U>
    inline bool operator<=(const InterpolatedValue<T>& a, const U& b)
    {
        return a == b || a < b;
    }

    template <typename T, typename U>
    inline bool operator>=(const InterpolatedValue<T>& a, const U& b)
    {
        return !(a < b);
    }

    template <typename T>
    constexpr InterpolatedValue<T> makeInterpolatedValue(const T& v)
    {
        return InterpolatedValue<T>(v);
    }
}
