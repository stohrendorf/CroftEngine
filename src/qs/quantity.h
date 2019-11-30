#pragma once

#include "serialization/serialization.h"

#include <string>
#include <type_traits>

namespace qs
{
template<typename, typename>
struct quantity;

namespace detail
{
template<typename T>
struct is_quantity
{
  static constexpr bool value = false;
};

template<typename A, typename B>
struct is_quantity<quantity<A, B>>
{
  static constexpr bool value = true;
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

  template<typename T>
  explicit quantity(T)
  {
    // must use T here, otherwise the static assert will trigger always
    static_assert(sizeof(T) > 0 && false, "Can only construct a quantity from its defined value type");
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
  constexpr std::enable_if_t<!detail::is_quantity<T>::value, quantity<unit, T>> retype_as() const
  {
    return quantity<Unit, T>{static_cast<T>(value)};
  }

  template<typename Q>
  constexpr std::enable_if_t<detail::is_quantity<Q>::value, quantity<unit, typename Q::type>> retype_as() const
  {
    static_assert(std::is_same<typename Q::unit, unit>::value, "Unit mismatch");
    return quantity<unit, typename Q::type>{static_cast<typename Q::type>(value)};
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

  void serialize(const serialization::Serializer& ser)
  {
    if(ser.loading)
    {
      Expects(ser.node.IsMap() && ser.node.size() == 2);
      Expects(ser.node["unit"].as<std::string>() == unit::suffix());
      value = ser.node["value"].as<type>();
    }
    else
    {
      ser.node["unit"] = unit::suffix();
      ser.node["value"] = value;
    }
  }

private:
  type value;
};

template<typename Unit, typename Type>
constexpr std::enable_if_t<std::is_signed<Type>::value, quantity<Unit, Type>> operator-(quantity<Unit, Type> l) noexcept
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
constexpr std::enable_if_t<std::is_signed<Type>::value, quantity<Unit, Type>>
  abs(const quantity<Unit, Type>& v) noexcept
{
  return v.get() >= 0 ? v : -v;
}

template<typename Type, typename Unit>
constexpr std::enable_if_t<!std::is_signed<Type>::value, quantity<Unit, Type>>
  abs(const quantity<Unit, Type>& v) noexcept
{
  return v;
}
} // namespace qs
