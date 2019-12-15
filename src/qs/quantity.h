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

template<typename Unit, typename Type>
struct quantity
{
  using unit = Unit;
  using type = Type;
  using self_type = quantity<unit, type>;

  template<typename T>
  using with_type = quantity<unit, T>;

  constexpr explicit quantity(type value = type{}) noexcept
      : value{value}
  {
  }

  constexpr quantity(const self_type& rhs) noexcept
      : value{rhs.value}
  {
  }

  constexpr self_type& operator=(const self_type& rhs) noexcept
  {
    if(&rhs != this)
      value = rhs.value;
    return *this;
  }

  template<typename T>
  explicit quantity(T)
  {
    // must use T here, otherwise the static assert will trigger always
    static_assert(sizeof(T) < 0,
                  "Can only construct a quantity from its defined value type"); // NOLINT(bugprone-sizeof-expression)
  }

  [[nodiscard]] std::string toString() const
  {
    return std::to_string(value) + Unit::suffix();
  }

  constexpr type get() const noexcept
  {
    return value;
  }

  template<typename T>
  constexpr auto get_as() const noexcept
  {
    return static_cast<T>(value);
  }

  template<typename T>
  constexpr auto retype_as() const
  {
    if constexpr(!detail::is_quantity<T>::value)
    {
      return quantity<Unit, T>{static_cast<T>(value)};
    }
    else
    {
      static_assert(std::is_same_v<typename T::unit, unit>, "Unit mismatch");
      return quantity<unit, typename T::type>{static_cast<typename T::type>(value)};
    }
  }

  constexpr self_type& operator+=(self_type r) noexcept
  {
    value += r.value;
    return *this;
  }

  constexpr self_type& operator-=(self_type r) noexcept
  {
    value -= r.value;
    return *this;
  }

  constexpr self_type& operator%=(self_type r) noexcept
  {
    value %= r.value;
    return *this;
  }

  constexpr self_type operator*=(type r) noexcept
  {
    value *= r;
    return *this;
  }

  template<typename T>
  constexpr self_type& operator*=(T r) = delete;

  constexpr self_type operator/=(type r) noexcept
  {
    value /= r;
    return *this;
  }

  template<typename T>
  constexpr self_type& operator/=(T r) = delete;

  // unary operator +
  constexpr self_type operator+() const noexcept
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
template<typename Type, typename Unit>
constexpr quantity<Unit, Type> abs(const quantity<Unit, Type>& v) noexcept
{
  if constexpr(std::is_signed_v<Type>)
    return v.get() >= 0 ? v : -v;
  else
    return v;
}
} // namespace qs
