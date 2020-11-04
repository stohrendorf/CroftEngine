#pragma once

#include <string>
#include <type_traits>

namespace qs
{
template<typename, typename>
struct quantity;

namespace detail
{
template<typename T>
struct is_quantity : std::false_type
{
};

template<typename A, typename B>
struct is_quantity<quantity<A, B>> : std::true_type
{
};
} // namespace detail

template<typename T>
inline constexpr bool is_quantity_v = detail::is_quantity<T>::value;

template<typename Unit, typename Type>
struct quantity // NOLINT: performance-unnecessary-value-param
{
  using unit = Unit;
  using type = Type;
  using self_type = quantity<unit, type>;

  template<typename T>
  using as_type = quantity<unit, T>;

  constexpr explicit quantity(type value = type{}) noexcept
      : value{value}
  {
  }

  constexpr quantity(const quantity<unit, type>& rhs) noexcept
      : value{rhs.value}
  {
  }

  constexpr quantity<unit, type>& operator=(const self_type& rhs) noexcept
  {
    if(&rhs != this)
      value = rhs.value;
    return *this;
  }

  template<typename T>
  explicit quantity(T)
  {
    // must use T here, otherwise the static assert will trigger always
    // cppcheck-suppress unsignedLessThanZero
    static_assert(sizeof(T) < 0, // NOLINT(bugprone-sizeof-expression)
                  "Can only construct a quantity from its defined value type");
  }

  [[nodiscard]] std::string toString() const
  {
    return std::to_string(value) + Unit::suffix();
  }

  template<typename T = type>
  constexpr auto get() const noexcept
  {
    return static_cast<T>(value);
  }

  template<typename T>
  constexpr auto cast() const
  {
    if constexpr(!is_quantity_v<T>)
    {
      return quantity<Unit, T>{get<T>()};
    }
    else
    {
      static_assert(std::is_same_v<typename T::unit, unit>, "Unit mismatch");
      return quantity<unit, typename T::type>{static_cast<typename T::type>(value)};
    }
  }

  constexpr auto& operator+=(self_type r) noexcept
  {
    value += r.value;
    return *this;
  }

  constexpr auto& operator-=(self_type r) noexcept
  {
    value -= r.value;
    return *this;
  }

  constexpr auto& operator%=(self_type r) noexcept
  {
    value %= r.value;
    return *this;
  }

  constexpr auto operator*=(type r) noexcept
  {
    value *= r;
    return *this;
  }

  template<typename T>
  constexpr auto& operator*=(T r) = delete;

  constexpr auto operator/=(type r) noexcept
  {
    value /= r;
    return *this;
  }

  template<typename T>
  constexpr auto& operator/=(T r) = delete;

  // unary operator +
  constexpr auto operator+() const noexcept
  {
    return *this;
  }

  // comparison operators
  constexpr bool operator<(self_type r) const noexcept
  {
    return value < r.value;
  }

  constexpr bool operator<=(self_type r) const noexcept
  {
    return value <= r.value;
  }

  constexpr bool operator==(self_type r) const noexcept
  {
    return value == r.value;
  }

  constexpr bool operator>(self_type r) const noexcept
  {
    return value > r.value;
  }

  constexpr bool operator>=(self_type r) const noexcept
  {
    return value >= r.value;
  }

  constexpr bool operator!=(self_type r) const noexcept
  {
    return value != r.value;
  }

private:
  type value;
};

template<typename Unit, typename Type>
constexpr std::enable_if_t<std::is_signed_v<Type>, quantity<Unit, Type>> operator-(quantity<Unit, Type> l) noexcept
{
  return quantity<Unit, Type>{static_cast<Type>(-l.get())};
}

template<typename Unit, typename Type>
constexpr auto operator*(Type l, quantity<Unit, Type> r) noexcept
{
  return quantity<Unit, Type>{static_cast<Type>(l * r.get())};
}

// abs
template<typename Unit, typename Type>
constexpr quantity<Unit, Type> abs(const quantity<Unit, Type>& v) noexcept
{
  if constexpr(std::is_signed_v<Type>)
    return v.get() >= 0 ? v : -v;
  else
    return v;
}
} // namespace qs
