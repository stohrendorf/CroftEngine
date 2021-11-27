#pragma once

#include <algorithm>
#include <gsl/gsl-lite.hpp>

namespace core
{
template<typename T>
struct Interval final
{
  T min{};
  T max{};

  explicit Interval() = default;
  Interval(const Interval<T>&) = default;
  Interval(Interval<T>&&) noexcept = default;

  Interval(const T& min, const T& max)
      : min{min}
      , max{max}
  {
  }

  constexpr auto& operator=(const Interval<T>& rhs)
  {
    min = rhs.min;
    max = rhs.max;
    return *this;
  }

  [[nodiscard]] constexpr bool operator==(const Interval<T>& rhs)
  {
    return min == rhs.min && max == rhs.max;
  }

  [[nodiscard]] constexpr auto operator+(const T& value) const
  {
    return Interval<T>{min + value, max + value};
  }

  constexpr auto& operator+=(const T& value)
  {
    min += value;
    max += value;
    return *this;
  }

  [[nodiscard]] constexpr auto operator-(const T& value) const
  {
    return Interval<T>{min - value, max - value};
  }

  [[nodiscard]] constexpr bool isValid() const
  {
    return min <= max;
  }

  [[nodiscard]] constexpr bool contains(const T& value) const
  {
    return min <= value && value <= max;
  }

  [[nodiscard]] constexpr bool containsExclusive(const T& value) const
  {
    return min < value && value < max;
  }

  [[nodiscard]] constexpr auto clamp(const T& value) const
  {
    return std::clamp(value, min, max);
  }

  [[nodiscard]] constexpr auto size() const
  {
    return max - min;
  }

  [[nodiscard]] constexpr auto mid() const
  {
    Expects(isValid());
    return (min + max) / 2;
  }

  [[nodiscard]] constexpr bool intersects(const Interval<T>& rhs) const
  {
    return min <= rhs.max && rhs.min <= max;
  }

  [[nodiscard]] constexpr bool intersectsExclusive(const Interval<T>& rhs) const
  {
    return min < rhs.max && rhs.min < max;
  }

  [[nodiscard]] constexpr auto narrowed(const T& value) const
  {
    return Interval<T>{min + value, max - value};
  }

  [[nodiscard]] constexpr auto broadened(const T& value) const
  {
    return narrowed(-value);
  }

  [[nodiscard]] constexpr auto intersect(const Interval<T>& rhs) const
  {
    return Interval<T>{std::max(min, rhs.min), std::min(max, rhs.max)};
  }

  [[nodiscard]] constexpr auto sanitized() const
  {
    return Interval<T>{std::min(min, max), std::max(min, max)};
  }
};

template<typename T>
constexpr auto operator+(const T& lhs, const Interval<T>& rhs)
{
  return Interval<T>{lhs + rhs.min, lhs + rhs.max};
}
} // namespace core
